--[[
Copyright 2011 by Anton Kochkov <anton.kochkov@gmail.com>

This file is part of Bvim.

Bvim is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Bvim is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Bvim.  If not, see <http://www.gnu.org/licenses/>.
--]]


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
