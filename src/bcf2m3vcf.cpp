
#include "vcf_reader.hpp"
#include "m3vcf_reader.hpp"

int main(int argc, char** argv)
{

  std::ofstream ofs(argv[2], std::ios::binary);

  vc::vcf::block buff;
  vc::vcf::reader input(argv[1]);
  vc::vcf::reader::input_iterator eof;
  vc::vcf::reader::input_iterator cur(input, buff);

  std::vector<std::string> sample_ids(input.samples_end() - input.samples_begin());
  std::copy(input.samples_begin(), input.samples_end(), sample_ids.begin());
  vc::m3vcf::writer compact_output(ofs, "20", 2, sample_ids.begin(), sample_ids.end());

  vc::m3vcf::block output_block(sample_ids.size(), 2);
  while (cur != eof)
  {
    if (!output_block.add_marker(cur->pos(), cur->ref(), cur->alt(), cur->begin(), cur->end()))
    {
      compact_output << output_block;
      output_block = vc::m3vcf::block(sample_ids.size(), 2);
      output_block.add_marker(cur->pos(), cur->ref(), cur->alt(), cur->begin(), cur->end());
    }

    ++cur;
  }
  if (output_block.marker_count())
    compact_output << output_block;

  return 0;
}