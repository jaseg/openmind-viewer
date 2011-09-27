#!/usr/bin/env python
##################################################
# Gnuradio Python Flow Graph
# Title: Top Block
# Generated: Tue Sep 27 16:52:55 2011
##################################################

from gnuradio import eng_notation
from gnuradio import gr
from gnuradio.eng_option import eng_option
from gnuradio.gr import firdes
from gnuradio.wxgui import scopesink2
from grc_gnuradio import wxgui as grc_wxgui
from optparse import OptionParser
import openmind
import wx
import os

#print 'Blocked waiting for GDB attach (pid = %d)' % (os.getpid(),)
#raw_input ('Press Enter to continue: ')

class top_block(grc_wxgui.top_block_gui):

	def __init__(self):
		grc_wxgui.top_block_gui.__init__(self, title="Top Block")

		##################################################
		# Variables
		##################################################
		self.samp_rate = samp_rate = 32000

		##################################################
		# Blocks
		##################################################
		self.gr_interleaved_short_to_complex_0 = gr.interleaved_short_to_complex()
		self.openmind_buspirate_source_0 = openmind.make_openmind_buspirate_source("/dev/ttyUSB0")
		self.wxgui_scopesink2_0 = scopesink2.scope_sink_c(
			self.GetWin(),
			title="Scope Plot",
			sample_rate=samp_rate,
			v_scale=0,
			v_offset=0,
			t_scale=0,
			ac_couple=False,
			xy_mode=False,
			num_inputs=1,
		)
		self.Add(self.wxgui_scopesink2_0.win)

		##################################################
		# Connections
		##################################################
		self.connect((self.gr_interleaved_short_to_complex_0, 0), (self.wxgui_scopesink2_0, 0))
		self.openmind_buspirate_source_0.test("foo")
		self.connect((self.openmind_buspirate_source_0, 0), (self.gr_interleaved_short_to_complex_0, 0))
		self.openmind_buspirate_source_0.test("baz")

	def set_samp_rate(self, samp_rate):
		self.samp_rate = samp_rate
		self.wxgui_scopesink2_0.set_sample_rate(self.samp_rate)

if __name__ == '__main__':
	parser = OptionParser(option_class=eng_option, usage="%prog: [options]")
	(options, args) = parser.parse_args()
	tb = top_block()
	tb.Run(True)

