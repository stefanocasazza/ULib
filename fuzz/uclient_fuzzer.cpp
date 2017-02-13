/**
 * uclient_fuzzer.cpp
 *
 * This file implements a test harness for userver with LibFuzzer.
 *
 * Running the fuzzers (example):
 * ----------------------------------------------------------------------------------
 * cd ../tests/examples; ./web_server.sh; cd ../../fuzz
 * ASAN_OPTIONS=detect_leaks=0 ./uclient_fuzzer -max_len=$((16 * 1024 )) http1-corpus
 * ----------------------------------------------------------------------------------
 *
 * See http://llvm.org/docs/LibFuzzer.html for more info
 */

#include <ulib/file_config.h>
#include <ulib/net/client/http.h>

#define MARKER "\n--MARK--\n"

static UHttpClient<USSLSocket>* client;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size) // Entry point for libfuzzer
{
	if (Size == 0)
		{
		U_INTERNAL_ASSERT_EQUALS(client, 0)

		// Perform one-time initialization

		U_ULIB_INIT(0);

		UFileConfig cfg;

		cfg.UFile::setPath(U_STRING_FROM_CONSTANT("uclient.cfg"));

		client = new UHttpClient<USSLSocket>(&cfg);

		client->setFollowRedirects(cfg.readBoolean(U_CONSTANT_TO_PARAM("FOLLOW_REDIRECTS")));
		client->setRequestPasswordAuthentication(cfg.at(U_CONSTANT_TO_PARAM("USER")), cfg.at(U_CONSTANT_TO_PARAM("PASSWORD_AUTH")));

		client->socket->setSSLActive(cfg.at(U_CONSTANT_TO_PARAM("CA_FILE")));

		return (client->connect() == false);
		}

	U_TRACE(5, "::LLVMFuzzerTestOneInput(%.*S,%u)", Size, Data, Size)

	U_INTERNAL_ASSERT_MAJOR(Size, 0)
	U_INTERNAL_ASSERT_POINTER(client)

	// Send fuzzed req and read results

	UString req;
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
		(void) req.assign(p1, len);

		if (binary == false &&
			 req.findEndHeader(U_CONSTANT_SIZE("GET /")) == false)
			{
				  if (UStringExt::endsWith(req, U_CONSTANT_TO_PARAM("\n\n")))			  req = UStringExt::dos2unix(req, true);
			else if (UStringExt::endsWith(req, U_CONSTANT_TO_PARAM("\r\n\r"))) (void) req.append(U_CONSTANT_TO_PARAM("\n"));
			else if (UStringExt::endsWith(req, U_CONSTANT_TO_PARAM("\r\n")))	 (void) req.append(U_CONSTANT_TO_PARAM("\r\n"));
			else if (UStringExt::endsWith(req, U_CONSTANT_TO_PARAM("\r")))		 (void) req.append(U_CONSTANT_TO_PARAM("\n\r\n"));
																									 (void) req.append(U_CONSTANT_TO_PARAM("\r\n\r\n"));
			}

		client->prepareRequest(req);

		if (client->sendRequest())
			{
			/*
			bool include = true; // include the HTTP-header in the output. The HTTP-header includes things like server-name, date of the document, HTTP-version...

			UString result = (include ? client->getResponse()
											  : client->getContent());

			if (result)
				{
				if (UStringExt::isGzip(result)) result = UStringExt::gunzip(result);

				(void) write(1, U_STRING_TO_PARAM(result));
				}
			*/
			}

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
