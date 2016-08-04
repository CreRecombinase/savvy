#include "m3vcf_reader.hpp"

#include <list>
#include <cmath>
#include <assert.h>
#include <arpa/inet.h>

std::uint64_t ceil_divide(std::uint64_t dividend, std::uint64_t divisor)
{
  return (std::uint64_t)(1) + ((dividend - (std::uint64_t)(1)) / divisor);
}

namespace vc
{
  namespace m3vcf
  {
    //================================================================//
    marker::marker(block& parent, std::uint32_t offset, const std::string& chromosome, std::uint64_t position, const std::string& ref, const std::string& alt)
      :
      parent_(parent),
      offset_(offset),
      chromosome_(chromosome),
      position_(position),
      ref_(ref),
      alt_(alt)
    {
    }

    const allele_status& marker::operator[](std::size_t i) const
    {
      return parent_.sample_haplotype_at(offset_, i);
    }

    std::uint64_t marker::haplotype_count() const
    {
      return parent_.haplotype_count();
    }

    double marker::calculate_allele_frequency() const
    {
      return parent_.calculate_allele_frequency(offset_);
    }
    //================================================================//

    //================================================================//
    const allele_status block::const_has_ref = allele_status::has_ref;
    const allele_status block::const_has_alt = allele_status::has_alt;
    const allele_status block::const_is_missing = allele_status::is_missing;

    const allele_status& block::sample_haplotype_at(std::uint32_t marker_off, std::uint64_t haplotype_off)
    {
      switch (unique_haplotype_matrix_[(marker_off * unique_haplotype_cnt_) + sample_mappings_[haplotype_off]])
      {
      case '0': return const_has_ref;
      case '1': return const_has_alt;
      case '.': return const_is_missing;
      };
      return const_is_missing;
    }

    const allele_status& block::unique_haplotype_at(std::uint32_t marker_offset, std::uint64_t unique_haplotype_offset)
    {
      switch (unique_haplotype_matrix_[(marker_offset * unique_haplotype_cnt_) + unique_haplotype_offset])
      {
      case '0': return const_has_ref;
      case '1': return const_has_alt;
      case '.': return const_is_missing;
      };
      return const_is_missing;
    }

    double block::calculate_allele_frequency(std::uint32_t marker_off) const
    {
      std::uint64_t allele_cnt = 0;
      std::uint64_t total_haplotypes = sample_size_ * ploidy_level_;

      for (std::uint32_t i = 0; i < unique_haplotype_cnt_; ++i)
      {
        char hap = unique_haplotype_matrix_[(marker_off * unique_haplotype_cnt_) + i];
        if (hap == '1')
          allele_cnt += haplotype_weights_[i];
        else if (hap != '0') // missing
          total_haplotypes -= haplotype_weights_[i];
      }

      return static_cast<double>(allele_cnt) / static_cast<double>(total_haplotypes);
    }

    block::const_iterator block::begin()
    {
      return const_iterator(this->markers_.data());
    }

    block::const_iterator block::end()
    {
      return const_iterator(this->markers_.data() + this->markers_.size());
    }

    const marker& block::operator[](std::size_t i) const
    {
      return markers_[i];
    }

    bool block::add_marker(std::uint64_t position, const std::string& ref, const std::string& alt, const char* hap_array, std::size_t hap_array_sz)
    {
      bool ret = false;

      if (hap_array_sz == sample_mappings_.size())
      {
        std::int64_t current_savings = static_cast<std::int64_t>(hap_array_sz * markers_.size()) - static_cast<std::int64_t>(unique_haplotype_cnt_ * markers_.size() + hap_array_sz);

        std::list<std::string> new_unique_haps;

        std::string column_string(markers_.size() + 1, '\0');
        std::vector<std::uint32_t> new_sample_mappings(hap_array_sz);

        for (std::size_t i = 0; i < hap_array_sz; ++i)
        {
          std::size_t j = 0;
          for (; j < markers_.size(); ++j)
          {
            column_string[j] = unique_haplotype_matrix_[(j * unique_haplotype_cnt_) + sample_mappings_[i]];
          }
          column_string[j] = hap_array[i];

          std::uint32_t unique_hap_offset = 0;
          auto find_it = new_unique_haps.begin();
          while (find_it != new_unique_haps.end())
          {
            if (*find_it == column_string)
              break;
            ++find_it;
            ++unique_hap_offset;
          }

          if (find_it == new_unique_haps.end())
            new_unique_haps.push_back(column_string);

          new_sample_mappings[i] = unique_hap_offset;
        }

        std::int64_t new_savings = static_cast<std::int64_t>(hap_array_sz * (markers_.size() + 1)) - static_cast<std::int64_t>(new_unique_haps.size() * (markers_.size() + 1) + hap_array_sz);
        if (new_savings >= current_savings)
        {
          markers_.push_back(marker(*this, markers_.size(), "", position, ref, alt));
          std::vector<char> new_unique_haplotype_matrix(new_unique_haps.size() * markers_.size());

          std::size_t i = 0;
          for (auto it = new_unique_haps.begin(); it != new_unique_haps.end(); ++it,++i)
          {
            for (std::size_t j = 0; j < it->size(); ++j)
            {
              new_unique_haplotype_matrix[(j * new_unique_haps.size()) + i] = (*it)[j];
            }
          }

          unique_haplotype_cnt_ = new_unique_haps.size();
          unique_haplotype_matrix_ = std::move(new_unique_haplotype_matrix);
          sample_mappings_ = std::move(new_sample_mappings);

          ret = true;
        }
      }

      return ret;
    }

    bool block::write_block(std::ostream& destination, block& source)
    {
      bool ret = true;

      std::uint32_t row_length;
      std::uint32_t column_length;
      assert(source.markers_.size() <= 0xFFFFFFFF);
      row_length = htonl(source.markers_.size());
      column_length = htonl(source.unique_haplotype_cnt_);
      destination.write((char*)&row_length, sizeof(row_length));
      destination.write((char*)&column_length, sizeof(column_length));

      //long byte_width_needed = static_cast<long>(std::ceil(std::log2(source.unique_haplotype_cnt_ + 1) / 8));
      std::uint8_t byte_width_needed = 4;
      if (source.unique_haplotype_cnt_ <= 0x100)
        byte_width_needed = 1;
      else if (source.unique_haplotype_cnt_ <= 0x10000)
        byte_width_needed = 2;

      std::vector<char> buff(source.haplotype_count() * byte_width_needed);
      for (std::size_t i = 0; i < source.haplotype_count(); ++i)
      {
        switch (byte_width_needed)
        {
          case 1:
          {
            std::memcpy(&buff[i], &(source.sample_mappings_[i]), 1);
            break;
          }
          case 2:
          {
            std::uint16_t nbo16 = htons(source.sample_mappings_[i]);
            std::memcpy(&buff[i * 2], &nbo16, 2);
            break;
          }
          default:
          {
            std::uint32_t nbo32 = htonl(source.sample_mappings_[i]);
            std::memcpy(&buff[i * 4], &nbo32, 4);
          }
        }
      }

      destination.write(buff.data(), buff.size());


      return ret;
    }
    //================================================================//

    //================================================================//
    reader::reader(std::istream& input_stream)
      :
      input_stream_(input_stream)
    {

    }

    bool reader::read_next_block(block& destination)
    {
      return block::read_block(destination, input_stream_);
    }
    //================================================================//
  }
}