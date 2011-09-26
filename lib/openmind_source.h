/* -*- c++ -*- */
/*
 * Copyright 2004 Free Software Foundation, Inc.
 * 
 * This file is part of GNU Radio
 * 
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */
#ifndef INCLUDED_OPENMIND_SOURCE_H
#define INCLUDED_OPENMIND_SOURCE_H

#include <gr_sync_block.h>

class openmind_source;

/*
 * We use boost::shared_ptr's instead of raw pointers for all access
 * to gr_blocks (and many other data structures).  The shared_ptr gets
 * us transparent reference counting, which greatly simplifies storage
 * management issues.  This is especially helpful in our hybrid
 * C++ / Python system.
 *
 * See http://www.boost.org/libs/smart_ptr/smart_ptr.htm
 *
 * As a convention, the _sptr suffix indicates a boost::shared_ptr
 */
typedef boost::shared_ptr<openmind_source> openmind_source_sptr;

/*!
 * \brief Return a shared_ptr to a new instance of myBlock.
 *
 * To avoid accidental use of raw pointers, myBlock's
 * constructor is private.  myBlock is the public
 * interface for creating new instances.
 */
openmind_source_sptr make_openmind_source (int param1);

/*!
 * \brief amplify a stream of floats.
 * \ingroup block
 *
 * This uses the preferred technique: subclassing gr_sync_block.
 */
class openmind_source : public gr_sync_block
{
private:
  // The friend declaration allows myBlock to
  // access the private constructor.

  friend openmind_source_sptr make_openmind_source (int param1);  

  int d_param1;

  openmind_source (int param1);  	// private constructor  

 public:
  int param1 () const { return d_param1; }
  void set_param1 (int param1) { d_param1 = param1; }

  ~openmind_source ();	// public destructor

  // Where all the action really happens

  int work (int noutput_items,
	    gr_vector_const_void_star &input_items,
	    gr_vector_void_star &output_items);
  bool start();
};

#endif /* INCLUDED_OPENMIND_SOURCE_H */
