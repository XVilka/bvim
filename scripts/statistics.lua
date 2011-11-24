-- Show all CRC* sums in tools window
function stats(block_start, block_end)
	bvi.block_select(155, block_start, block_end, 0)
	bvi.tools_window(4)
	bvi.print_tools_window("Block #155 [" .. block_start .. "," .. block_end .. "]", 1)
	bvi.print_tools_window("------------------------------------------------", 2)
	bvi.print_tools_window("Entropy: " .. stat.entropy(155), 3)
end


