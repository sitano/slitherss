#include "game/sector.h"

#include <algorithm>

void BoundBox::Insert(Sector *s) {
  auto fwd_i = std::lower_bound(sectors.begin(), sectors.end(), s);
  if (fwd_i != sectors.end()) {
    sectors.insert(fwd_i, s);
  } else {
    sectors.push_back(s);
  }
}

bool BoundBox::RemoveUnsorted(const SectorIter &i) {
  if (i + 1 != sectors.end()) {
    *i = sectors.back();
    sectors.pop_back();
    return true;
  } else {
    sectors.pop_back();
    return false;
  }
}

bool BoundBox::IsPresent(const Sector *s) {
  return std::binary_search(sectors.begin(), sectors.end(), s);
}

void BoundBox::Sort() { std::sort(sectors.begin(), sectors.end()); }

size_t BoundBox::get_sectors_count() { return sectors.size(); }

size_t BoundBox::get_snakes_in_sectors_count() {
  size_t i = 0;
  for (Sector *s : sectors) {
    i += s->snakes.size();
  }
  return i;
}

void Sector::RemoveSnake(snake_id_t id) {
  snakes.erase(std::remove_if(
      snakes.begin(), snakes.end(),
      [id](const BoundBox *bb) { return bb->id == id; }), snakes.end());
}

void Sector::Insert(const Food &f) {
  auto fwd_i = std::lower_bound(
      food.begin(), food.end(), f,
      [](const Food &a, const Food &b) { return a.x < b.x; });

  if (fwd_i != food.end()) {
    food.insert(fwd_i, f);
  } else {
    food.push_back(f);
  }
}

void Sector::Remove(const FoodSeqIter &i) {
  food.erase(i);
}

void Sector::Sort() {
  std::sort(food.begin(), food.end(),
            [](const Food &a, const Food &b) { return a.x < b.x; });
}

FoodSeqIter Sector::FindClosestFood(uint16_t fx) {
  return std::lower_bound(
      food.begin(), food.end(), Food{fx, 0, 0, 0},
      [](const Food &a, const Food &b) { return a.x < b.x; });
}

void SectorSeq::InitSectors() {
  const size_t len = WorldConfig::sector_count_along_edge *
                     WorldConfig::sector_count_along_edge;
  reserve(len);
  for (size_t i = 0; i < len; i++) {
    push_back(Sector{
        static_cast<uint8_t>(i % WorldConfig::sector_count_along_edge),
        static_cast<uint8_t>(i / WorldConfig::sector_count_along_edge)});
  }
}

size_t SectorSeq::get_index(const uint16_t x, const uint16_t y) {
  return y * WorldConfig::sector_count_along_edge + x;
}

Sector *SectorSeq::get_sector(const uint16_t x, const uint16_t y) {
  return &operator[](get_index(x, y));
}

void SnakeBoundBox::InsertSortedWithReg(Sector *s) {
  Insert(s);
  s->snakes.push_back(this);
}

void ViewPort::RegNewSectorIfMissing(Sector *s) {
  if (std::find(new_sectors.begin(), new_sectors.end(), s) == new_sectors.end()) {
    new_sectors.push_back(s);
  }
}

void ViewPort::RegOldSectorIfMissing(Sector *s) {
  if (std::find(old_sectors.begin(), old_sectors.end(), s) == old_sectors.end()) {
    old_sectors.push_back(s);
  }
}

void ViewPort::InsertSortedWithDelta(Sector *s) {
  Insert(s);
  RegNewSectorIfMissing(s);
}

void SnakeBoundBox::UpdateBoxNewSectors(SectorSeq *ss, const float bb_r,
                                        const float new_x, const float new_y,
                                        const float old_x, const float old_y) {
  const int16_t new_sx = static_cast<int16_t>(new_x / WorldConfig::sector_size);
  const int16_t new_sy = static_cast<int16_t>(new_y / WorldConfig::sector_size);
  const int16_t old_sx = static_cast<int16_t>(old_x / WorldConfig::sector_size);
  const int16_t old_sy = static_cast<int16_t>(old_y / WorldConfig::sector_size);
  if (new_sx == old_sx && new_sy == old_sy) {
    return;
  }

  const BoundBoxPos box = {new_x, new_y, bb_r};
  static const int16_t map_width_sectors =
      static_cast<int16_t>(WorldConfig::sector_count_along_edge);

  for (int j = new_sy - 1; j <= new_sy + 1; j++) {
    for (int i = new_sx - 1; i <= new_sx + 1; i++) {
      if (i >= 0 && i < map_width_sectors && j >= 0 && j < map_width_sectors) {
        Sector *new_sector = ss->get_sector(i, j);
        if (!IsPresent(new_sector) && new_sector->Intersect(box)) {
          InsertSortedWithReg(new_sector);
        }
      }
    }
  }
}

void SnakeBoundBox::UpdateBoxOldSectors() {
  const size_t prev_len = sectors.size();
  auto i = sectors.begin();
  auto sec_end = sectors.end();
  while (i != sec_end) {
    Sector *sec = *i;
    if (!sec->Intersect(*this)) {
      sec->RemoveSnake(id);
      if (RemoveUnsorted(i)) {
        sec_end = sectors.end();
        continue;
      } else {
        break;
      }
    }
    i++;
  }

  if (prev_len != sectors.size()) {
    Sort();
  }
}

void ViewPort::UpdateBoxNewSectors(SectorSeq *ss,
                                   const float new_x, const float new_y,
                                   const float old_x, const float old_y) {
  const int16_t new_sx = static_cast<int16_t>(new_x / WorldConfig::sector_size);
  const int16_t new_sy = static_cast<int16_t>(new_y / WorldConfig::sector_size);
  const int16_t old_sx = static_cast<int16_t>(old_x / WorldConfig::sector_size);
  const int16_t old_sy = static_cast<int16_t>(old_y / WorldConfig::sector_size);
  if (new_sx == old_sx && new_sy == old_sy) {
    return;
  }

  static const int16_t map_width_sectors =
      static_cast<int16_t>(WorldConfig::sector_count_along_edge);

  for (int j = new_sy - 3; j <= new_sy + 3; j++) {
    for (int i = new_sx - 3; i <= new_sx + 3; i++) {
      if (i >= 0 && i < map_width_sectors && j >= 0 && j < map_width_sectors) {
        Sector *new_sector = ss->get_sector(i, j);
        if (!IsPresent(new_sector) && new_sector->Intersect(*this)) {
          InsertSortedWithDelta(new_sector);
        }
      }
    }
  }
}

void ViewPort::UpdateBoxOldSectors() {
  const size_t prev_len = sectors.size();
  auto i = sectors.begin();
  auto sec_end = sectors.end();
  while (i != sec_end) {
    Sector *sec = *i;
    if (!sec->Intersect(*this)) {
      RegOldSectorIfMissing(sec);
      if (RemoveUnsorted(i)) {
        sec_end = sectors.end();
        continue;
      } else {
        break;
      }
    }
    i++;
  }

  if (prev_len != sectors.size()) {
    Sort();
  }
}
