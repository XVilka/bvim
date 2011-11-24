-- Show all CRC* sums in tools window
function stats(block_start, block_end)
	bvim.block_select(155, block_start, block_end, 0)
	bvim.tools_window(4)
	bvim.print_tools_window("Block #155 [" .. block_start .. "," .. block_end .. "]", 1)
	bvim.print_tools_window("------------------------------------------------", 2)
	bvim.print_tools_window("Entropy: " .. stat.entropy(155), 3)
end


