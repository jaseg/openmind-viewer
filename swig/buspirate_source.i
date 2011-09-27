/* -*- c++ -*- */

GR_SWIG_BLOCK_MAGIC(openmind,buspirate_source);

openmind_buspirate_source_sptr make_openmind_buspirate_source (std::string device);

class openmind_buspirate_source : public gr_sync_block
{
private:
  std::string d_device;
  openmind_buspirate_source (std::string device);  

public:
  std::string device () const { return d_device; }

};
