tests_passed = 0
tests_failed = 0

if hash.md4("1234567890") ~= "85b196c3e39457d91cab9c905f9a11c0" then
	bvim.msg_window("MD4 hash is WRONG!")
	bvim.tools_window(2)
	bvim.print_tools_window("BVI  MD4 hash: [ " .. hash.md4("1234567890") .. " ]", 1)
	bvim.print_tools_window("ORIG MD4 hash: [ 85b196c3e39457d91cab9c905f9a11c0 ]", 2)
	tests_failed = tests_failed + 1
else
	tests_passed = tests_passed + 1
end
if hash.md5("1234567890") ~= "e807f1fcf82d132f9bb018ca6738a19f"  then
	bvim.msg_window("MD5 hash is WRONG!")
	bvim.tools_window(2)
	bvim.print_tools_window("BVI  MD5 hash: [ " .. hash.md5("1234567890") .. " ]", 1)
	bvim.print_tools_window("ORIG MD5 hash: [ e807f1fcf82d132f9bb018ca6738a19f ]", 2)
	tests_failed = tests_failed + 1
else
	tests_passed = tests_passed + 1
end
if hash.sha1("1234567890") ~= "01b307acba4f54f55aafc33bb06bbbf6ca803e9a" then
	bvim.msg_window("SHA1 hash is WRONG!")
	bvim.tools_window(2)
	bvim.print_tools_window("BVI  SHA1 hash: [ " .. hash.sha1("1234567890") .. " ]", 1)
	bvim.print_tools_window("ORIG SHA1 hash: [ 01b307acba4f54f55aafc33bb06bbbf6ca803e9a ]", 2)
	tests_failed = tests_failed + 1
else
	tests_passed = tests_passed + 1
end
if hash.sha256("1234567890") ~= "c775e7b757ede630cd0aa1113bd102661ab38829ca52a6422ab782862f268646" then
	bvim.msg_window("SHA256 hash is WRONG!")
	bvim.tools_window(2)
	bvim.print_tools_window("BVI  SHA256 hash: [ " .. hash.sha256("1234567890") .. " ]", 1)
	bvim.print_tools_window("ORIG SHA256 hash: [ c775e7b757ede630cd0aa1113bd102661ab38829ca52a6422ab782862f268646 ]", 2)
	tests_failed = tests_failed + 1
else
	tests_passed = tests_passed + 1
end
if hash.sha512("1234567890") ~= "12b03226a6d8be9c6e8cd5e55dc6c7920caaa39df14aab92d5e3ea9340d1c8a4d3d0b8e4314f1f6ef131ba4bf1ceb9186ab87c801af0d5c95b1befb8cedae2b9" then
	bvim.msg_window("SHA512 hash is WRONG!")
	bvim.tools_window(2)
	bvim.print_tools_window("BVI  SHA512 hash: [ " .. hash.sha512("1234567890") .. " ]", 1)
	bvim.print_tools_window("ORIG SHA512 hash: [ 12b03226a6d8be9c6e8cd5e55dc6c7920caaa39df14aab92d5e3ea9340d1c8a4d3d0b8e4314f1f6ef131ba4bf1ceb9186ab87c801af0d5c95b1befb8cedae2b9 ]", 2)
	tests_failed = tests_failed + 1
else
	tests_passed = tests_passed + 1
end
if hash.ripemd160("1234567890") ~= "9d752daa3fb4df29837088e1e5a1acf74932e074" then
	bvim.msg_window("RIPEMD160 hash is WRONG!")
	bvim.tools_window(2)
	bvim.print_tools_window("BVI  RIPEMD160 hash: [ " .. hash.ripemd160("1234567890") .. " ]", 1)
	bvim.print_tools_window("ORIG RIPEMD160 hash: [ 9d752daa3fb4df29837088e1e5a1acf74932e074 ]", 2)
	tests_failed = tests_failed + 1
else
	tests_passed = tests_passed + 1
end

bvim.msg_window("PASSED: " .. tests_passed .. " FAILED: " .. tests_failed)
