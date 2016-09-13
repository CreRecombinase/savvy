
#include "cvcf_reader.hpp"
#include "m3vcf_reader.hpp"
#include "vcf_reader.hpp"
#include "test_class.hpp"
#include "varint.hpp"
#include "vc.hpp"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <sstream>
#include <tuple>
#include <type_traits>
#include <utility>

bool has_extension(const std::string& fullString, const std::string& ext)
{
  if (fullString.length() >= ext.length())
    return (0 == fullString.compare (fullString.length() - ext.length(), ext.length(), ext));
  else
    return false;
}

template <typename T>
void handle_file_reader(T& reader)
{
  typename T::input_iterator::buffer buff;
  typename T::input_iterator end;
  typename T::input_iterator it(reader, buff);

  while (it != end)
  {
    double af = it->calculate_allele_frequency();
    for (auto jt = it->begin(); jt != it->end(); ++jt)
    {
      vc::allele_status foo = *jt;
    }

    std::for_each(it->begin(), it->end(), [](const typename vc::allele_status& s)
    {
      vc::allele_status foo = s;
    });

    ++it;
  }

}

int reader_tests()
{
  //----------------------------------------------------------------//
  {
    std::string file_path = "/foobar.cvcf";
    if (has_extension(file_path, ".cvcf"))
    {
      std::ifstream ifs("/foobar.cvcf");
      vc::cvcf::reader input(ifs);
      handle_file_reader(input);
    }
    else if (has_extension(file_path, ".m3vcf"))
    {
      std::ifstream ifs("/foobar.m3vcf");
      vc::m3vcf::reader input(ifs);
      handle_file_reader(input);
    }
    else if (has_extension(file_path, ".vcf") || has_extension(file_path, "vcf.gz") || has_extension(file_path, ".bcf"))
    {
      vc::vcf::block buff;
      vc::vcf::reader input(file_path);
      vc::vcf::reader::input_iterator eof;
      vc::vcf::reader::input_iterator cur(input, buff);

      while (cur != eof)
        ++cur;
    }
  }
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  {
    std::string file_path = "/foobar.cvcf";
    if (has_extension(file_path, ".cvcf"))
    {
      std::ifstream ifs("/foobar.cvcf");
      vc::cvcf::reader input(ifs);
      auto analysis = make_analysis(input);
      analysis.run();
    }
    else
    {
      std::ifstream ifs("/foobar.m3vcf");
      vc::m3vcf::reader input(ifs);
      some_analysis<vc::m3vcf::reader> analysis(input);
      analysis.run();
    }
  }
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  {
    std::ifstream ifs("/foobar.cvcf");
    vc::cvcf::reader input(ifs);
    vc::cvcf::marker buff;

    for (vc::cvcf::reader::input_iterator i(input, buff), eof; i != eof; ++i)
    {
      for (auto j = i->begin(); j != i->end(); ++j)
      {

      }
    }
  }
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  {
    std::ifstream ifs("/foobar.m3vcf");
    vc::m3vcf::reader input(ifs);
    vc::m3vcf::block buff;

    std::for_each(vc::m3vcf::reader::input_iterator(input, buff), vc::m3vcf::reader::input_iterator(), [](const vc::m3vcf::marker& m)
    {
      std::for_each(m.begin(), m.end(), [](const vc::allele_status& s)
      {

      });
    });
  }
  //----------------------------------------------------------------//

  //----------------------------------------------------------------//
  vc::cvcf::marker m;
  std::uint64_t ploidy_level = 2;
  std::uint64_t sample_size = 1000;
  std::vector<int> zero_one_two_vec(sample_size, 0);

  std::for_each(m.non_ref_begin(), m.non_ref_end(), [&zero_one_two_vec, ploidy_level](const vc::cvcf::marker::sparse_vector_allele& a)
  {
    if (a.status == vc::allele_status::has_alt)
      ++(zero_one_two_vec[a.offset / ploidy_level]);
  });

  std::size_t i = 0;
  std::for_each(m.begin(), m.end(), [&zero_one_two_vec, &i, ploidy_level](const vc::allele_status& s)
  {
    if (s == vc::allele_status::has_alt)
      ++(zero_one_two_vec[i / ploidy_level]);
  });

  i = 0;
  for (const auto& s : m)
  {
    if (s == vc::allele_status::has_alt)
      ++(zero_one_two_vec[i / ploidy_level]);
  }
  //----------------------------------------------------------------//
  return 0;
}

int varint_test()
{
  std::vector<std::uint64_t> arr(0xFFFFFF);
  for (std::uint64_t i = 0; i < arr.size(); ++i)
    arr[i] = i;
  std::cout << std::accumulate(arr.begin(), arr.end(), 0ULL) << std::endl;

  {
    std::ofstream non_compressed_arr_ostream("foo-not-compressed.bin", std::ios::binary);
    const auto encode_start = std::chrono::high_resolution_clock::now();
    non_compressed_arr_ostream.write((char*)arr.data(), arr.size() * sizeof(std::uint64_t));
    non_compressed_arr_ostream.flush();
    //std::copy(arr.begin(), arr.end(), back_it);
    auto encode_elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - encode_start).count();
    std::cout << "Encode elapsed time: " << encode_elapsed_time << "ms" << std::endl;
    non_compressed_arr_ostream.close();

    std::fill(arr.begin(), arr.end(), 0);

    std::ifstream non_compressed_arr_istream("foo-not-compressed.bin", std::ios::binary);
    const auto decode_start = std::chrono::high_resolution_clock::now();
    non_compressed_arr_istream.read((char*)arr.data(), arr.size() * sizeof(std::uint64_t));
//    for (std::size_t i = 0; i < arr.size(); ++i)
//      arr[i] = ntohll(arr[i]);
    auto decode_elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - decode_start).count();
    std::cout << "Non-compressed copy: " << std::accumulate(arr.begin(), arr.end(), 0ULL) << std::endl;
    std::cout << "Decode elapsed time: " << decode_elapsed_time << "ms" << std::endl;
    std::cout << std::endl;
  }

  {
    std::ofstream compressed_arr_ostream("foo-0bit.bin");
    const auto encode_start = std::chrono::high_resolution_clock::now();
    std::ostreambuf_iterator<char> output_it(compressed_arr_ostream);
    for (std::uint64_t i = 0; i < arr.size(); ++i)
      vc::varint_encode(i, output_it);
    compressed_arr_ostream.flush();
    auto encode_elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - encode_start).count();
    std::cout << "Encode elapsed time: " << encode_elapsed_time << "ms" << std::endl;
    compressed_arr_ostream.close();

    std::fill(arr.begin(), arr.end(), 0);

    std::ifstream compressed_arr_istream("foo-0bit.bin");
    const auto decode_start = std::chrono::high_resolution_clock::now();
    std::istreambuf_iterator<char> decode_it(compressed_arr_istream);
    for (std::uint64_t i = 0; i < arr.size(); ++i)
      decode_it = ++vc::varint_decode(decode_it, std::istreambuf_iterator<char>(), arr[i]);
    auto decode_elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - decode_start).count();
    std::cout << "0-bit prefixed: " << std::accumulate(arr.begin(), arr.end(), 0ULL) << std::endl;
    std::cout << "Decode elapsed time: " << decode_elapsed_time << "ms" << std::endl;
    compressed_arr_istream.close();
    std::cout << std::endl;
  }

  {
    std::uint8_t prefix_data = 0;
    std::ofstream compressed_arr_ostream("foo-1bit.bin");
    const auto encode_start = std::chrono::high_resolution_clock::now();
    std::ostreambuf_iterator<char> output_it(compressed_arr_ostream);
    for (std::uint64_t i = 0; i < arr.size(); ++i)
      vc::one_bit_prefixed_varint::encode(prefix_data, i, output_it);
    compressed_arr_ostream.flush();
    auto encode_elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - encode_start).count();
    std::cout << "Encode elapsed time: " << encode_elapsed_time << "ms" << std::endl;
    compressed_arr_ostream.close();

    std::fill(arr.begin(), arr.end(), 0);

    std::ifstream compressed_arr_istream("foo-1bit.bin");
    const auto decode_start = std::chrono::high_resolution_clock::now();
    std::istreambuf_iterator<char> decode_it(compressed_arr_istream);
    for (std::uint64_t i = 0; i < arr.size(); ++i)
      decode_it = ++vc::one_bit_prefixed_varint::decode(decode_it, std::istreambuf_iterator<char>(), prefix_data, arr[i]);
    auto decode_elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - decode_start).count();
    std::cout << "1-bit prefixed: " << std::accumulate(arr.begin(), arr.end(), 0ULL) << std::endl;
    std::cout << "Decode elapsed time: " << decode_elapsed_time << "ms" << std::endl;
    compressed_arr_istream.close();
    std::cout << std::endl;
  }

  {
    std::uint8_t prefix_data = 0;
    std::ofstream compressed_arr_ostream("foo-2bit.bin");
    const auto encode_start = std::chrono::high_resolution_clock::now();
    std::ostreambuf_iterator<char> output_it(compressed_arr_ostream);
    for (std::uint64_t i = 0; i < arr.size(); ++i)
      vc::two_bit_prefixed_varint::encode(prefix_data, i, output_it);
    compressed_arr_ostream.flush();
    auto encode_elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - encode_start).count();
    std::cout << "Encode elapsed time: " << encode_elapsed_time << "ms" << std::endl;
    compressed_arr_ostream.close();

    std::fill(arr.begin(), arr.end(), 0);

    std::ifstream compressed_arr_istream("foo-2bit.bin");
    const auto decode_start = std::chrono::high_resolution_clock::now();
    std::istreambuf_iterator<char> decode_it(compressed_arr_istream);
    for (std::uint64_t i = 0; i < arr.size(); ++i)
      decode_it = ++vc::two_bit_prefixed_varint::decode(decode_it, std::istreambuf_iterator<char>(), prefix_data, arr[i]);
    auto decode_elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - decode_start).count();
    std::cout << "2-bit prefixed: " << std::accumulate(arr.begin(), arr.end(), 0ULL) << std::endl;
    std::cout << "Decode elapsed time: " << decode_elapsed_time << "ms" << std::endl;
    compressed_arr_istream.close();
    std::cout << std::endl;
  }

  {
    std::uint8_t prefix_data = 0;
    std::ofstream compressed_arr_ostream("foo-3bit.bin");
    const auto encode_start = std::chrono::high_resolution_clock::now();
    std::ostreambuf_iterator<char> output_it(compressed_arr_ostream);
    for (std::uint64_t i = 0; i < arr.size(); ++i)
      vc::three_bit_prefixed_varint::encode(prefix_data, i, output_it);
    compressed_arr_ostream.flush();
    auto encode_elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - encode_start).count();
    std::cout << "Encode elapsed time: " << encode_elapsed_time << "ms" << std::endl;
    compressed_arr_ostream.close();

    std::fill(arr.begin(), arr.end(), 0);

    std::ifstream compressed_arr_istream("foo-3bit.bin");
    const auto decode_start = std::chrono::high_resolution_clock::now();
    std::istreambuf_iterator<char> decode_it(compressed_arr_istream);
    for (std::uint64_t i = 0; i < arr.size(); ++i)
      decode_it = ++vc::three_bit_prefixed_varint::decode(decode_it, std::istreambuf_iterator<char>(), prefix_data, arr[i]);
    auto decode_elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - decode_start).count();
    std::cout << "3-bit prefixed: " << std::accumulate(arr.begin(), arr.end(), 0ULL) << std::endl;
    std::cout << "Decode elapsed time: " << decode_elapsed_time << "ms" << std::endl;
    compressed_arr_istream.close();
    std::cout << std::endl;
  }

  {
    std::uint8_t prefix_data = 0;
    std::ofstream compressed_arr_ostream("foo-4bit.bin");
    const auto encode_start = std::chrono::high_resolution_clock::now();
    std::ostreambuf_iterator<char> output_it(compressed_arr_ostream);
    for (std::uint64_t i = 0; i < arr.size(); ++i)
      vc::four_bit_prefixed_varint::encode(prefix_data, i, output_it);
    compressed_arr_ostream.flush();
    auto encode_elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - encode_start).count();
    std::cout << "Encode elapsed time: " << encode_elapsed_time << "ms" << std::endl;
    compressed_arr_ostream.close();

    std::fill(arr.begin(), arr.end(), 0);

    std::ifstream compressed_arr_istream("foo-4bit.bin");
    const auto decode_start = std::chrono::high_resolution_clock::now();
    std::istreambuf_iterator<char> decode_it(compressed_arr_istream);
    for (std::uint64_t i = 0; i < arr.size(); ++i)
      decode_it = ++vc::four_bit_prefixed_varint::decode(decode_it, std::istreambuf_iterator<char>(), prefix_data, arr[i]);
    auto decode_elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - decode_start).count();
    std::cout << "4-bit prefixed: " << std::accumulate(arr.begin(), arr.end(), 0ULL) << std::endl;
    std::cout << "Decode elapsed time: " << decode_elapsed_time << "ms" << std::endl;
    compressed_arr_istream.close();
    std::cout << std::endl;
  }

  {
    std::uint8_t prefix_data = 0;
    std::ofstream compressed_arr_ostream("foo-5bit.bin");
    const auto encode_start = std::chrono::high_resolution_clock::now();
    std::ostreambuf_iterator<char> output_it(compressed_arr_ostream);
    for (std::uint64_t i = 0; i < arr.size(); ++i)
      vc::five_bit_prefixed_varint::encode(prefix_data, i, output_it);
    compressed_arr_ostream.flush();
    auto encode_elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - encode_start).count();
    std::cout << "Encode elapsed time: " << encode_elapsed_time << "ms" << std::endl;
    compressed_arr_ostream.close();

    std::fill(arr.begin(), arr.end(), 0);

    std::ifstream compressed_arr_istream("foo-5bit.bin");
    const auto decode_start = std::chrono::high_resolution_clock::now();
    std::istreambuf_iterator<char> decode_it(compressed_arr_istream);
    for (std::uint64_t i = 0; i < arr.size(); ++i)
      decode_it = ++vc::five_bit_prefixed_varint::decode(decode_it, std::istreambuf_iterator<char>(), prefix_data, arr[i]);
    auto decode_elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - decode_start).count();
    std::cout << "5-bit prefixed: " << std::accumulate(arr.begin(), arr.end(), 0ULL) << std::endl;
    std::cout << "Decode elapsed time: " << decode_elapsed_time << "ms" << std::endl;
    compressed_arr_istream.close();
    std::cout << std::endl;
  }

  {
    std::uint8_t prefix_data = 0;
    std::ofstream compressed_arr_ostream("foo-6bit.bin");
    const auto encode_start = std::chrono::high_resolution_clock::now();
    std::ostreambuf_iterator<char> output_it(compressed_arr_ostream);
    for (std::uint64_t i = 0; i < arr.size(); ++i)
      vc::six_bit_prefixed_varint::encode(prefix_data, i, output_it);
    compressed_arr_ostream.flush();
    auto encode_elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - encode_start).count();
    std::cout << "Encode elapsed time: " << encode_elapsed_time << "ms" << std::endl;
    compressed_arr_ostream.close();

    std::fill(arr.begin(), arr.end(), 0);

    std::ifstream compressed_arr_istream("foo-6bit.bin");
    const auto decode_start = std::chrono::high_resolution_clock::now();
    std::istreambuf_iterator<char> decode_it(compressed_arr_istream);
    for (std::uint64_t i = 0; i < arr.size(); ++i)
      decode_it = ++vc::six_bit_prefixed_varint::decode(decode_it, std::istreambuf_iterator<char>(), prefix_data, arr[i]);
    auto decode_elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - decode_start).count();
    std::cout << "6-bit prefixed: " << std::accumulate(arr.begin(), arr.end(), 0ULL) << std::endl;
    std::cout << "Decode elapsed time: " << decode_elapsed_time << "ms" << std::endl;
    compressed_arr_istream.close();
    std::cout << std::endl;
  }

  {
    std::uint8_t prefix_data = 0;
    std::ofstream compressed_arr_ostream("foo-7bit.bin");
    const auto encode_start = std::chrono::high_resolution_clock::now();
    std::ostreambuf_iterator<char> output_it(compressed_arr_ostream);
    for (std::uint64_t i = 0; i < arr.size(); ++i)
      vc::seven_bit_prefixed_varint::encode(prefix_data, i, output_it);
    compressed_arr_ostream.flush();
    auto encode_elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - encode_start).count();
    std::cout << "Encode elapsed time: " << encode_elapsed_time << "ms" << std::endl;
    compressed_arr_ostream.close();

    std::fill(arr.begin(), arr.end(), 0);

    std::ifstream compressed_arr_istream("foo-7bit.bin");
    const auto decode_start = std::chrono::high_resolution_clock::now();
    std::istreambuf_iterator<char> decode_it(compressed_arr_istream);
    for (std::uint64_t i = 0; i < arr.size(); ++i)
      decode_it = ++vc::seven_bit_prefixed_varint::decode(decode_it, std::istreambuf_iterator<char>(), prefix_data, arr[i]);
    auto decode_elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - decode_start).count();
    std::cout << "7-bit prefixed: " << std::accumulate(arr.begin(), arr.end(), 0ULL) << std::endl;
    std::cout << "Decode elapsed time: " << decode_elapsed_time << "ms" << std::endl;
    compressed_arr_istream.close();
    std::cout << std::endl;
  }

  return 0;
}

void convert_file_test()
{
  {
    std::ofstream ofs("chr1.cvcf", std::ios::binary);

    vc::vcf::block buff;
    vc::vcf::reader input("chr1.bcf");
    vc::vcf::reader::input_iterator eof;
    vc::vcf::reader::input_iterator cur(input, buff);

//    std::cout << "CHROM\tPOS\tREF\tALT\t";
//    for (auto it = input.samples_begin(); it != input.samples_end(); ++it)
//    {
//      std::cout << *it << "_1" << "\t" << *it << "_2";
//      if (it + 1 < input.samples_end())
//        std::cout << "\t";
//    }
//    std::cout << std::endl;

    std::vector<std::string> sample_ids(input.samples_end() - input.samples_begin());
    std::copy(input.samples_begin(), input.samples_end(), sample_ids.begin());
    vc::cvcf::writer compact_output(ofs, "1", 2, sample_ids.begin(), sample_ids.end());


    while (cur != eof)
    {
//      std::cout << vc::vcf::reader::get_chromosome(input, *cur) << "\t" << cur->pos() << "\t" << cur->ref() << "\t" << cur->alt() << "\t";
//      for (auto gt = cur->begin(); gt != cur->end(); )
//      {
//        if (*gt == vc::allele_status::has_ref)
//          std::cout << "0";
//        else if (*gt == vc::allele_status::has_alt)
//          std::cout << "1";
//        else
//          std::cout << ".";
//
//        ++gt;
//        if (gt != cur->end())
//          std::cout << "\t";
//      }
//
//      std::cout << std::endl;

      compact_output << vc::cvcf::marker(cur->pos(), cur->ref(), cur->alt(), cur->begin(), cur->end());

      ++cur;
    }
  }

//  {
//    std::cout << std::endl << std::endl;
//    std::ifstream ifs("sample_conversion.m3vcf", std::ios::binary);
//    vc::m3vcf::reader compact_input(ifs);
//    vc::m3vcf::block buff;
//    vc::m3vcf::reader::input_iterator cur(compact_input, buff);
//    vc::m3vcf::reader::input_iterator end;
//
//    while (cur != end)
//    {
//      std::cout << "22" << "\t" << cur->pos() << "\t" << cur->ref() << "\t" << cur->alt() << "\t";
//
//      for (auto gt = cur->begin(); gt != cur->end(); )
//      {
//        if (*gt == vc::allele_status::has_ref)
//          std::cout << "0";
//        else if (*gt == vc::allele_status::has_alt)
//          std::cout << "1";
//        else
//          std::cout << ".";
//
//        ++gt;
//        if (gt != cur->end())
//          std::cout << "\t";
//      }
//
//      std::cout << std::endl;
//
//      ++cur;
//    }
//  }

}

template <typename Proc>
class timed_procedure_call
{
public:
  timed_procedure_call(Proc& procedure)
  {
    start_ = std::chrono::high_resolution_clock::now();
    return_value_ = procedure();
    end_ = std::chrono::high_resolution_clock::now();
  }
  bool return_value() const { return return_value_; }
  template <typename Duration>
  long long int elapsed_time()
  {
    return std::chrono::duration_cast<Duration>(end_ - start_).count();
  }
private:
  bool return_value_;
  std::chrono::high_resolution_clock::time_point start_;
  std::chrono::high_resolution_clock::time_point end_;
};

template <typename Proc>
timed_procedure_call<Proc> time_procedure(Proc& p)
{
  return timed_procedure_call<Proc>(p);
}

template <typename R1, typename R2>
class file_checksum_test
{
public:
  file_checksum_test(R1& reader1, R2& reader2) : reader1_(reader1), reader2_(reader2) {}
  bool operator()() const
  {
    std::size_t checksum1 = get_checksum(reader1_);
    std::size_t checksum2 = get_checksum(reader2_);

    std::cout << checksum1 << " " << checksum2 << std::endl;

    return checksum1 == checksum2;
  }
private:
  template <typename T>
  static std::size_t hash_combine(std::size_t seed, const T& val)
  {
    seed ^= std::hash<T>()(val) + 0x9e3779b9 + (seed<<6) + (seed>>2);
    return seed;
  }

  template <typename ReaderType>
  static std::size_t get_checksum(ReaderType& reader)
  {
    std::size_t ret = 0;

    typename ReaderType::input_iterator::buffer buff;
    typename ReaderType::input_iterator cur(reader, buff);
    typename ReaderType::input_iterator end;

    while (cur != end)
    {
      ret = std::hash<std::uint64_t>()(cur->pos()) ^ ret;
      ret = std::hash<std::string>()(cur->ref()) ^ ret;
      ret = std::hash<std::string>()(cur->alt()) ^ ret;

      for (auto gt = cur->begin(); gt != cur->end(); ++gt)
        ret = hash_combine(ret, static_cast<int>(*gt));

      ++cur;
    }

    return ret;
  }
  R1& reader1_;
  R2& reader2_;
};


class file_handler_functor
{
public:
  template <typename T>
  void operator()(T&& input_file_reader)
  {
    input_file_reader.sample_count();
  }
};

class triple_file_handler_functor
{
public:
  template <typename T, typename T2, typename T3>
  void operator()(T&& input_file_reader, T2&& input_file_reader2, T3&& input_file_reader3)
  {
    input_file_reader.sample_count();
    input_file_reader2.sample_count();
    input_file_reader3.sample_count();

    std::tuple<T, T2, T3> file_readers(std::move(input_file_reader), std::move(input_file_reader2), std::move(input_file_reader3));

  }
};

class marker_handler_functor
{
public:
  template <typename T>
  void operator()(const T& mrkr)
  {
    std::uint64_t pos = mrkr.pos();
    std::string ref = mrkr.ref();
    std::string alt = mrkr.alt();
    std::for_each(mrkr.begin(), mrkr.end(), [](const vc::allele_status& s)
    {

    });
  }
};

int main(int argc, char** argv)
{

  vc::open_marker_files(triple_file_handler_functor(), "chr1.bcf", "chr1.cvcf", "chr1.m3vcf");

  vc::open_marker_files(std::make_tuple("chr1.cvcf", "chr1.m3vcf"), [](auto&& input_file_reader1, auto&& input_file_reader2)
  {
    typedef typename std::remove_reference<decltype(input_file_reader1)>::type R1;
    typename R1::input_iterator::buffer buf{};
    typename R1::input_iterator eof{};
    typename R1::input_iterator it(input_file_reader1, buf);

    typedef typename std::remove_reference<decltype(input_file_reader2)>::type R2;
    typename R2::input_iterator::buffer buf2{};
    typename R2::input_iterator eof2{};
    typename R2::input_iterator it2(input_file_reader2, buf2);

    while (it != eof)
    {

      ++it;
    }

    while (it2 != eof2)
    {

      ++it2;
    }

  });

  vc::open_marker_file("chr1.bcf", [](auto&& input_file_reader1)
  {
    vc::open_marker_file("chr1.bcf", [&input_file_reader1](auto&& input_file_reader2)
    {
      typedef typename std::remove_reference<decltype(input_file_reader1)>::type R1;
      typename R1::input_iterator::buffer buf{};
      typename R1::input_iterator eof{};
      typename R1::input_iterator it(input_file_reader1, buf);

      typedef typename std::remove_reference<decltype(input_file_reader2)>::type R2;
      typename R2::input_iterator::buffer buf2{};
      typename R2::input_iterator eof2{};
      typename R2::input_iterator it2(input_file_reader2, buf2);

      while (it != eof)
      {

        ++it;
      }

      while (it2 != eof2)
      {

        ++it2;
      }

    });
  });

  vc::open_marker_file("chr1.bcf", file_handler_functor());
  file_handler_functor f;
  vc::open_marker_file("chr1.bcf", f);

  vc::iterate_marker_file("chr1.bcf", marker_handler_functor());

  return 0;

//  {
//    convert_file_test();
//  }

  {
    vc::vcf::reader bcf_reader("chr1.bcf");

    std::ifstream ifs("chr1.cvcf", std::ios::binary);
    vc::cvcf::reader cvcf_reader(ifs);

    file_checksum_test<vc::cvcf::reader, vc::vcf::reader> t(cvcf_reader, bcf_reader);

    std::cout << "Starting checksum test ..." << std::endl;
    auto timed_call = time_procedure(t);
    std::cout << "Returned: " << (timed_call.return_value() ? "True" : "FALSE") << std::endl;
    std::cout << "Elapsed Time: " << timed_call.elapsed_time<std::chrono::milliseconds>() << "ms" << std::endl;
  }

  return 0;
}


