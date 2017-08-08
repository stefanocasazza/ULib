/**
 * uhttp_parser_fuzzer.cpp
 *
 * This file implements a test harness for UHTTP parser with LibFuzzer
 *
 * See http://llvm.org/docs/LibFuzzer.html for more info
 */

#include <ulib/utility/uhttp.h>

#define MARKER "\n--MARK--\n"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size) // Entry point for libfuzzer
{
	if (Size == 0)
		{
		// Perform one-time initialization

		U_ULIB_INIT(0);

		u_init_ulib_hostname();

		UClientImage_Base::init();

		UString::str_allocate(STR_ALLOCATE_HTTP);

		return true;
		}

	U_TRACE(5, "::LLVMFuzzerTestOneInput(%.*S,%u)", Size, Data, Size)

	U_INTERNAL_ASSERT_MAJOR(Size, 0)

	bool binary;
	const char* p1;
	const char* p2;
	uint32_t len, cnt = 0, pos = 0;
	const char* pend = (const char*)Data+Size;

loop:
	p1 = (const char*)Data+pos;

	binary = (u__istext(*p1) == false);

	if (binary == false &&
		 u__isspace(*p1))
		{
		do {
			if (++p1 == pend) return 0;
			}
		while (u__isspace(*p1));
		}

	p2 = (const char*)memmem(p1, Size-pos, U_CONSTANT_TO_PARAM(MARKER));

	len = (p2 ? p2-p1 : pend-p1);

	U_INTERNAL_DUMP("len = %u binary = %u", len, binary)

	if (binary ||
		 len > U_CONSTANT_SIZE("GET /"))
		{
		UHTTP::parserExecute(p1, len);

		if (p2)
			{
			pos += len + U_CONSTANT_SIZE(MARKER);

			U_INTERNAL_DUMP("pos = %u cnt = %u", pos, cnt)

			if (pos < Size &&
				 cnt++ < 20)
				{
				goto loop;
				}
			}
		}

	return 0;
}
