-- Show all CRC* sums in tools window
function crcsums(block_start, block_end)
end

-- Show all MD* sums in tools window
function mdsums(block_start, block_end)
end

-- Show all SHA* sums in tools window
function shasums(block_start, block_end)
	bvi.block_select(0, block_start, block_end, 0)
	bvi.tools_window(4)
	bvi.print_tools_window("Block #0 [" .. block_start .. "," .. block_end .. "]", 1)
	bvi.print_tools_window("------------------------------------------------", 2)
	bvi.print_tools_window("SHA1   : " .. bvi.sha1_hash(0), 3)
	bvi.print_tools_window("SHA256 : " .. bvi.sha256_hash(0), 4)
	bvi.print_tools_window("SHA512 : " .. bvi.sha512_hash(0), 5)
end
