/* -*- c++ -*- */

GR_SWIG_BLOCK_MAGIC(openmind,openmind_source);

openmind_source_sptr make_openmind_source (int param1);

class myBlock : public gr_sync_block
{
private:
  int d_param1;
  openmind_source (int param1);  

public:
  int param1 () const { return d_param1; }
  void set_param1 (int param1) { d_param1 = param1; }

};
