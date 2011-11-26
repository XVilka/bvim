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
function stats(block_start, block_end)
	bvim.block_select(155, block_start, block_end, 0)
	bvim.tools_window(4)
	bvim.print_tools_window("Block #155 [" .. block_start .. "," .. block_end .. "]", 1)
	bvim.print_tools_window("------------------------------------------------", 2)
	bvim.print_tools_window("Entropy: " .. stat.entropy(155), 3)
end


