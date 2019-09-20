#include "stdafx.h"

#include <ida.hpp>
#include <idp.hpp>
#include <loader.hpp>
#include <xref.hpp> //xrefs
#include <allins.hpp> //mnemonics
#include <vector>

int IDAP_init(void)
{
	// Do checks here to ensure your plug-in is being used within
	// an environment it was written for. Return PLUGIN_SKIP if the
	// checks fail, otherwise return PLUGIN_KEEP.
	return PLUGIN_KEEP;
}
void IDAP_term(void)
{
	// Stuff to do when exiting, generally you'd put any sort
	// of clean-up jobs here.
	return;
}

std::vector<ea_t> FindFunctionCalls(const ea_t localAddress)
{
	std::vector<ea_t> functionCalls;
	xrefblk_t ref;

	for (bool res = ref.first_to(localAddress, XREF_FAR); res;
		res = ref.next_to())
	{
		if (ref.iscode)
		{
			char mnem[MAXSTR];
			// Fill the cmd structure with the disassembly of
			// the current address and get the mnemonic text.
			ua_mnem(ref.from, mnem, sizeof(mnem) - 1);

			if (cmd.itype == NN_call || cmd.itype == NN_callfi || cmd.itype == NN_callni)
				functionCalls.push_back(ref.from);
		}

	}

	return functionCalls;
}

std::vector<ea_t> FindInnerRets(func_t * curFunc)
{
	int movinstrs[] = { NN_retf, NN_retfd, NN_retfq, NN_retfw, NN_retn, NN_retnd, NN_retnq, NN_retnw, 0 };
	std::vector<ea_t> retns;

	for (ea_t addr = curFunc->startEA; addr < curFunc->endEA; addr++) {
		// Get the flags for this address
		flags_t flags = get_flags_novalue(addr);
		// Only look at the address if it's a head byte, i.e.
		// the start of an instruction and is code.
		if (isHead(flags) && isCode(flags)) {
			char mnem[MAXSTR];
			// Fill the cmd structure with the disassembly of
			// the current address and get the mnemonic text.
			ua_mnem(addr, mnem, sizeof(mnem) - 1);
			// Check the mnemonic of the address against all
			// mnemonics we're interested in.
			for (int i = 0; movinstrs[i] != 0; i++) {
				if (cmd.itype == movinstrs[i])
					retns.push_back(addr);
			}
		}
	}

	return retns;
}

// The plugin can be passed an integer argument from the plugins.cfg
// file. This can be useful when you want the one plug-in to do
// something different depending on the hot-key pressed or menu
// item selected.
void IDAP_run(int arg)
{
	// Loop through all segments
	for (int segm = 0; segm < get_segm_qty(); segm++)
	{
		segment_t *seg = getnseg(segm);

		// We need CODE segments only
		if (seg->type == SEG_CODE)
		{
			// Loop through all functions in segment
			for (int segFunction = 0; segFunction < get_func_qty(); ++segFunction)
			{
				// Returns a pointer to the func_t representing 
				// the function number supplied as seqFunction
				func_t *curFunc = getn_func(segFunction);
				ea_t localAddress = curFunc->startEA;
				
				if (localAddress == NULL)
					continue;

				auto functionCalls = FindFunctionCalls(localAddress);
				auto rets = FindInnerRets(curFunc);

				for (auto ret = rets.begin(); ret != rets.end(); ++ret)
				{
					for (auto call = functionCalls.begin(); call != functionCalls.end(); ++call)
					{
						add_cref(*call, *ret, fl_CF);
						msg("Xref from call %x to %x\n", *ret, *call);
					}
						
				}
			}
		}
	}

	return;
}

/*
void IDAP_run(int arg)
{
	xrefblk_t xb;
	ea_t addr = get_screen_ea();
	// Replicate IDA 'x' keyword functionality
	for (bool res = xb.first_to(addr, XREF_FAR); res;
		res = xb.next_to()) {
		char buf[MAXSTR];
		char clean_buf[MAXSTR];
		// Get the disassembly text for the referencing addr
		generate_disasm_line(xb.from, buf, sizeof(buf) - 1);
		// Clean out any format or colour codes
		tag_remove(buf, clean_buf, sizeof(clean_buf) - 1);
		msg("%a: %s\n", xb.from, clean_buf);
	}
}
*/
// There isn't much use for these yet, but I set them anyway.
char IDAP_comment[] = "This is my test plug-in";
char IDAP_help[] = "Cross reference";

// The name of the plug-in displayed in the Edit->Plugins menu. It
//can // be overridden in the user's plugins.cfg file.
char IDAP_name[] = "Cross reference";

// The hot-key the user can use to run your plug-in.
char IDAP_hotkey[] = "Alt-X";

// The all-important exported PLUGIN object
plugin_t PLUGIN =
{
	IDP_INTERFACE_VERSION, // IDA version plug-in is written for
	0, // Flags (see below)
	IDAP_init, // Initialisation function
	IDAP_term, // Clean-up function
	IDAP_run, // Main plug-in body
	IDAP_comment, // Comment – unused
	IDAP_help, // As above – unused
	IDAP_name, // Plug-in name shown in
	// Edit->Plugins menu
	IDAP_hotkey
};
