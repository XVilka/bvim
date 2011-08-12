bvi.exec("set color(data,999,000,000)")
bvi.display_error("here is an example error message!")
bvi.msg_window("please, press any key to exit window")
bvi.scrolldown(5)
bvi.print(8, 15, 4, "here is lua scripting test")
bvi.exec("block 0 12 233 4");
bvi.exec("block 1 357 683 2");
bvi.exec("block 2 749 919 3");
bvi.block_xor(0, 15);
bvi.block_and(1, 14);
bvi.block_or(2, 255);
bvi.block_lshift(1, 2);
bvi.block_and(2, 26);
bvi.block_rrotate(2, 3);
bvi.tools_window(4);
bvi.print_tools_window("Block #0", 1);
bvi.print_tools_window("--------", 2);
bvi.print_tools_window("SHA1   :" .. bvi.sha1_hash(0), 3);
bvi.print_tools_window("SHA256 :" .. bvi.sha256_hash(0), 4);
