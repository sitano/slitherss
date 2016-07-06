#ifndef SLITHER_SERVER_STREAMBUF_ARRAY_HPP
#define SLITHER_SERVER_STREAMBUF_ARRAY_HPP

#include <array>
#include <boost/asio.hpp>
#include <limits>

template <std::size_t _Nm = 128>
class streambuf_array : public std::streambuf, private boost::noncopyable {
 public:
  /// The type used to represent the input sequence as a list of buffers.
  typedef boost::asio::const_buffers_1 const_buffers_type;
  /// The type used to represent the output sequence as a list of buffers.
  typedef boost::asio::mutable_buffers_1 mutable_buffers_type;

  /// Construct a streambuf_array object.
  /**
   * Constructs a streambuf with the specified maximum size. The initial size
   * of the streambuf's input sequence is 0.
   */
  explicit streambuf_array() : max_size_(_Nm) {
    const std::size_t pend = max_size_;
    setg(&buffer_[0], &buffer_[0], &buffer_[0]);
    setp(&buffer_[0], &buffer_[0] + pend);
  }

  /// Get the size of the input sequence.
  /**
   * @returns The size of the input sequence. The value is equal to that
   * calculated for @c s in the following code:
   * @code
   * size_t s = 0;
   * const_buffers_type bufs = data();
   * const_buffers_type::const_iterator i = bufs.begin();
   * while (i != bufs.end())
   * {
   *   const_buffer buf(*i++);
   *   s += buffer_size(buf);
   * }
   * @endcode
   */
  std::size_t size() const { return pptr() - gptr(); }

  /// Get the maximum size of the streambuf_array.
  /**
   * @returns The allowed maximum of the sum of the sizes of the input sequence
   * and output sequence.
   */
  std::size_t max_size() const { return max_size_; }

  /// Get a list of buffers that represents the input sequence.
  /**
   * @returns An object of type @c const_buffers_type that satisfies
   * ConstBufferSequence requirements, representing all character arrays in the
   * input sequence.
   *
   * @note The returned object is invalidated by any @c streambuf_array member
   * function that modifies the input sequence or output sequence.
   */
  const_buffers_type data() const {
    return boost::asio::buffer(boost::asio::const_buffer(
        gptr(), (pptr() - gptr()) * sizeof(char_type)));
  }

  /// Get a list of buffers that represents the output sequence, with the given
  /// size.
  /**
   * Ensures that the output sequence can accommodate @c n characters,
   * reallocating character array objects as necessary.
   *
   * @returns An object of type @c mutable_buffers_type that satisfies
   * MutableBufferSequence requirements, representing character array objects
   * at the start of the output sequence such that the sum of the buffer sizes
   * is @c n.
   *
   * @throws std::length_error If <tt>size() + n > max_size()</tt>.
   *
   * @note The returned object is invalidated by any @c streambuf_array member
   * function that modifies the input sequence or output sequence.
   */
  mutable_buffers_type prepare(std::size_t n) {
    reserve(n);
    return boost::asio::buffer(
        boost::asio::mutable_buffer(pptr(), n * sizeof(char_type)));
  }

  /// Move characters from the output sequence to the input sequence.
  /**
   * Appends @c n characters from the start of the output sequence to the input
   * sequence. The beginning of the output sequence is advanced by @c n
   * characters.
   *
   * Requires a preceding call <tt>prepare(x)</tt> where <tt>x >= n</tt>, and
   * no intervening operations that modify the input or output sequence.
   *
   * @note If @c n is greater than the size of the output sequence, the entire
   * output sequence is moved to the input sequence and no error is issued.
   */
  void commit(std::size_t n) {
    if (pptr() + n > epptr()) n = epptr() - pptr();
    pbump(static_cast<int>(n));
    setg(eback(), gptr(), pptr());
  }

  /// Remove characters from the input sequence.
  /**
   * Removes @c n characters from the beginning of the input sequence.
   *
   * @note If @c n is greater than the size of the input sequence, the entire
   * input sequence is consumed and no error is issued.
   */
  void consume(std::size_t n) {
    if (egptr() < pptr()) setg(&buffer_[0], gptr(), pptr());
    if (gptr() + n > pptr()) n = pptr() - gptr();
    gbump(static_cast<int>(n));
  }

 protected:
  /// Override std::streambuf behaviour.
  /**
   * Behaves according to the specification of @c std::streambuf::underflow().
   */
  int_type underflow() {
    if (gptr() < pptr()) {
      setg(&buffer_[0], gptr(), pptr());
      return traits_type::to_int_type(*gptr());
    } else {
      return traits_type::eof();
    }
  }

  /// Override std::streambuf behaviour.
  /**
   * Behaves according to the specification of @c std::streambuf::overflow(),
   * with the specialisation that @c std::length_error is thrown if appending
   * the character to the input sequence would require the condition
   * <tt>size() > max_size()</tt> to be true.
   */
  int_type overflow(int_type c) {
    if (!traits_type::eq_int_type(c, traits_type::eof())) {
      if (pptr() == epptr()) {
        const std::size_t gnext = gptr() - &buffer_[0];
        if (gnext > 0) {
          reserve(gnext);
        } else {
          std::length_error ex("::streambuf_array too short");
          boost::asio::detail::throw_exception(ex);
        }
      }

      *pptr() = traits_type::to_char_type(c);
      pbump(1);
      return c;
    }

    return traits_type::not_eof(c);
  }

  void reserve(std::size_t n) {
    // Get current stream positions as offsets.
    std::size_t gnext = gptr() - &buffer_[0];
    std::size_t pnext = pptr() - &buffer_[0];
    std::size_t pend = epptr() - &buffer_[0];

    // Check if there is already enough space in the put area.
    if (n <= pend - pnext) {
      return;
    }

    // Shift existing contents of get area to start of buffer.
    if (gnext > 0) {
      pnext -= gnext;
      std::memmove(&buffer_[0], &buffer_[0] + gnext, pnext);
    }

    // Ensure buffer is large enough to hold at least the specified size.
    if (n > pend - pnext) {
      if (n <= max_size_ && pnext <= max_size_ - n) {
        std::length_error ex("::streambuf_array too short");
        boost::asio::detail::throw_exception(ex);
      } else {
        std::length_error ex("boost::asio::streambuf too long");
        boost::asio::detail::throw_exception(ex);
      }
    }
  }

 private:
  std::size_t max_size_;
  std::array<char_type, _Nm> buffer_;

  // Helper function to get the preferred size for reading data.
  friend std::size_t read_size_helper(streambuf_array &sb,
                                      std::size_t max_size) {
    return std::min<std::size_t>(
        std::max<std::size_t>(512, sb.buffer_.max_size() - sb.size()),
        std::min<std::size_t>(max_size, sb.max_size() - sb.size()));
  }
};

// Helper function to get the preferred size for reading data. Used for any
// user-provided specialisations of streambuf_array.
template <std::size_t _Nm>
inline std::size_t read_size_helper(streambuf_array<_Nm> &sb,
                                    std::size_t max_size) {
  return std::min<std::size_t>(
      512, std::min<std::size_t>(max_size, sb.max_size() - sb.size()));
}

#endif  // SLITHER_SERVER_STREAMBUF_ARRAY_HPP
