#!/usr/bin/env python
##################################################
# Gnuradio Python Flow Graph
# Title: Top Block
# Generated: Thu Oct 13 21:55:05 2011
##################################################

from gnuradio import eng_notation
from gnuradio import gr
from gnuradio.eng_option import eng_option
from gnuradio.gr import firdes
from gnuradio.wxgui import scopesink2
from grc_gnuradio import wxgui as grc_wxgui
from optparse import OptionParser
import wx

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
		self.wxgui_scopesink2_0 = scopesink2.scope_sink_f(
			self.GetWin(),
			title="Scope Plot",
			sample_rate=125,
			v_scale=0,
			v_offset=0,
			t_scale=1000,
			ac_couple=False,
			xy_mode=False,
			num_inputs=1,
			trig_mode=gr.gr_TRIG_MODE_AUTO,
			y_axis_label="Counts",
		)
		self.Add(self.wxgui_scopesink2_0.win)
		self.gr_wavfile_sink_0 = gr.wavfile_sink("/tmp/openmind_dump_ch0.wav", 1, 125, 16)
		self.gr_short_to_float_0_0 = gr.short_to_float()
		self.gr_file_source_0 = gr.file_source(gr.sizeof_short*1, "/tmp/openmind_ch0", True)

		##################################################
		# Connections
		##################################################
		self.connect((self.gr_short_to_float_0_0, 0), (self.wxgui_scopesink2_0, 0))
		self.connect((self.gr_file_source_0, 0), (self.gr_short_to_float_0_0, 0))
		self.connect((self.gr_short_to_float_0_0, 0), (self.gr_wavfile_sink_0, 0))

	def get_samp_rate(self):
		return self.samp_rate

	def set_samp_rate(self, samp_rate):
		self.samp_rate = samp_rate

if __name__ == '__main__':
	parser = OptionParser(option_class=eng_option, usage="%prog: [options]")
	(options, args) = parser.parse_args()
	tb = top_block()
	tb.Run(True)

