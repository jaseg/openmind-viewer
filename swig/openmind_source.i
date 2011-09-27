/* -*- c++ -*- */

GR_SWIG_BLOCK_MAGIC(openmind,openmind_source);

openmind_source_sptr make_openmind_source (string device);

class openmind_source : public gr_sync_block
{
private:
  string d_device;
  openmind_source (string device);  

public:
  string device () const { return d_device; }

};
