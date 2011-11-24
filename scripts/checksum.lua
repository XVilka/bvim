-- Show all CRC* sums in tools window
function crcsums(block_start, block_end)
	bvim.block_select(14, block_start, block_end, 0)
	bvim.tools_window(4)
	bvim.print_tools_window("Block #14 [" .. block_start .. "," .. block_end .. "]", 1)
	bvim.print_tools_window("------------------------------------------------", 2)
	bvim.print_tools_window("CRC-8  : [ " .. "<not yet implemented>" .. " ]", 3)
	bvim.print_tools_window("CRC-16 : [ " .. string.gsub(string.format("%04x", hash.crc16(14)), "(..)(..)", "%1 %2") .. " ]", 4)
	bvim.print_tools_window("CRC-32 : [ " .. string.gsub(string.format("%08x", hash.crc32(14)), "(..)(..)(..)(..)", "%1 %2 %3 %4") .. " ]", 5)
end

-- Show all MD* sums in tools window
function mdsums(block_start, block_end)
	bvim.block_select(14, block_start, block_end, 0)
	bvim.tools_window(3)
	bvim.print_tools_window("Block #14 [" .. block_start .. "," .. block_end .. "]", 1)
	bvim.print_tools_window("------------------------------------------------", 2)
	bvim.print_tools_window("MD4 : " .. hash.md4(14), 3)
	bvim.print_tools_window("MD5 : " .. hash.md5(14), 4)
end

-- Show all SHA* sums in tools window
function shasums(block_start, block_end)
	bvim.block_select(14, block_start, block_end, 0)
	bvim.tools_window(4)
	bvim.print_tools_window("Block #14 [" .. block_start .. "," .. block_end .. "]", 1)
	bvim.print_tools_window("------------------------------------------------", 2)
	bvim.print_tools_window("SHA1   : " .. hash.sha1(14), 3)
	bvim.print_tools_window("SHA256 : " .. hash.sha256(14), 4)
	bvim.print_tools_window("SHA512 : " .. hash.sha512(14), 5)
end
