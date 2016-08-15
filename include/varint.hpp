#ifndef LIBVC_VARINT_HPP
#define LIBVC_VARINT_HPP

#include <cstdint>
#include <string>
#include <iterator>
#include <cmath>


namespace vc
{
  //----------------------------------------------------------------//
  template <std::uint8_t PrefixMask, std::uint8_t ContinueFlagForFirstByte>
  class prefixed_varint
  {
  public:
    template <typename OutputIt>
    static void encode(std::uint8_t prefix_data, std::uint64_t input, OutputIt& output_it)
    {
      prefix_data = prefix_data & (std::uint8_t)PrefixMask;
      prefix_data |= (std::uint8_t)(first_byte_integer_value_mask & input);
      input >>= initial_bits_to_shift;
      if (input)
        prefix_data |= ContinueFlagForFirstByte;
      *output_it = prefix_data;
      ++output_it;

      while (input)
      {
        std::uint8_t next_byte = static_cast<std::uint8_t>(input & 0x7F);
        input >>= 7;

        if (input)
          next_byte |= 0x80;

        *output_it = next_byte;
        ++output_it;
      }
    }

    template <typename InputIt>
    static InputIt decode(InputIt input_it, const InputIt end_it, std::uint8_t& prefix_data, std::uint64_t& output)
    {
      output = 0;
      if (input_it != end_it)
      {
        std::uint8_t current_byte = static_cast<std::uint8_t>(*input_it);
        prefix_data = (std::uint8_t)(current_byte & PrefixMask);
        output |= (current_byte & first_byte_integer_value_mask);

        if (current_byte & ContinueFlagForFirstByte)
        {
          ++input_it;
          std::uint8_t bits_to_shift = initial_bits_to_shift;
          while (input_it != end_it)
          {
            current_byte = static_cast<std::uint8_t>(*input_it);
            output |= (std::uint64_t) (current_byte & 0x7F) << bits_to_shift;
            if ((current_byte & 0x80) == 0)
              break;
            ++input_it;
            bits_to_shift += 7;
          }
        }
      }

      return input_it;
    }
  private:
    static const std::uint8_t first_byte_integer_value_mask;
    static const std::uint8_t initial_bits_to_shift;
    prefixed_varint() = delete;
  };

  typedef prefixed_varint<0x80, 0x40> one_bit_prefixed_varint;
  typedef prefixed_varint<0xC0, 0x20> two_bit_prefixed_varint;
  typedef prefixed_varint<0xE0, 0x10> three_bit_prefixed_varint;
  typedef prefixed_varint<0xF0,  0x8> four_bit_prefixed_varint;
  typedef prefixed_varint<0xF8,  0x4> five_bit_prefixed_varint;
  typedef prefixed_varint<0xFC,  0x2> six_bit_prefixed_varint;
  typedef prefixed_varint<0xFE,  0x1> seven_bit_prefixed_varint;
  //----------------------------------------------------------------//



  //----------------------------------------------------------------//
  template <typename OutputIt>
  void varint_encode(std::uint64_t input, OutputIt& output_it)
  {
    do
    {
      std::uint8_t next_byte = static_cast<std::uint8_t>(input & 0x7F);
      input >>= 7;

      if (input)
        next_byte |= 0x80;

      *output_it = next_byte;
      ++output_it;
    } while (input);
  }
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  template <typename InputIt>
  InputIt varint_decode(InputIt input_it, const InputIt end_it, std::uint64_t& output)
  {
    output = 0;
    std::uint8_t current_byte;

    std::uint8_t bits_to_shift = 0;
    while (input_it != end_it)
    {
      current_byte = static_cast<std::uint8_t>(*input_it);
      output |= (std::uint64_t) (current_byte & 0x7F) << bits_to_shift;
      if ((current_byte & 0x80) == 0)
        break;
      ++input_it;
      bits_to_shift += 7;
    }

    return input_it;
  }
  //----------------------------------------------------------------//
}

#endif //LIBVC_VARINT_HPP