-- Show all CRC* sums in tools window
function crcsums(block_start, block_end)
	bvi.block_select(14, block_start, block_end, 0)
	bvi.tools_window(4)
	bvi.print_tools_window("Block #14 [" .. block_start .. "," .. block_end .. "]", 1)
	bvi.print_tools_window("------------------------------------------------", 2)
	bvi.print_tools_window("CRC-8  : [ " .. "<not yet implemented>" .. " ]", 3)
	bvi.print_tools_window("CRC-16 : [ " .. string.gsub(string.format("%04x", bvi.crc16(14)), "(..)(..)", "%1 %2") .. " ]", 4)
	bvi.print_tools_window("CRC-32 : [ " .. string.gsub(string.format("%08x", bvi.crc32(14)), "(..)(..)(..)(..)", "%1 %2 %3 %4") .. " ]", 5)
end

-- Show all MD* sums in tools window
function mdsums(block_start, block_end)
	bvi.block_select(14, block_start, block_end, 0)
	bvi.tools_window(3)
	bvi.print_tools_window("Block #14 [" .. block_start .. "," .. block_end .. "]", 1)
	bvi.print_tools_window("------------------------------------------------", 2)
	bvi.print_tools_window("MD4 : " .. bvi.md4_hash(14), 3)
	bvi.print_tools_window("MD5 : " .. bvi.md5_hash(14), 4)
end

-- Show all SHA* sums in tools window
function shasums(block_start, block_end)
	bvi.block_select(14, block_start, block_end, 0)
	bvi.tools_window(4)
	bvi.print_tools_window("Block #14 [" .. block_start .. "," .. block_end .. "]", 1)
	bvi.print_tools_window("------------------------------------------------", 2)
	bvi.print_tools_window("SHA1   : " .. bvi.sha1_hash(14), 3)
	bvi.print_tools_window("SHA256 : " .. bvi.sha256_hash(14), 4)
	bvi.print_tools_window("SHA512 : " .. bvi.sha512_hash(14), 5)
end
