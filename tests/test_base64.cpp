#include <paraviewo/base64Layer.hpp>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

namespace
{
	std::string reference_base64(const std::string &data)
	{
		constexpr char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		std::string result;
		unsigned int bits = 0;
		int count = 0;
		for (const unsigned char byte : data)
		{
			bits = (bits << 8) | byte;
			count += 8;
			while (count >= 6)
			{
				count -= 6;
				result += alphabet[(bits >> count) & 63];
			}
		}
		if (count > 0)
			result += alphabet[(bits << (6 - count)) & 63];
		while (result.size() % 4 != 0)
			result += '=';
		return result;
	}
} // namespace

TEST_CASE("Base64 preserves arbitrary streaming boundaries", "[base64]")
{
	for (const std::size_t length : {0, 1, 2, 3, 4, 5, 6, 7, 8, 255, 256, 257, 49151, 49152, 49153, 65536, 131075})
	{
		std::string data(length, '\0');
		for (std::size_t i = 0; i < length; ++i)
			data[i] = static_cast<char>((i * 37 + 19) & 255);
		for (const std::size_t chunk : {1, 2, 3, 7, 4096, 65536})
		{
			CAPTURE(length, chunk);
			std::ostringstream stream;
			paraviewo::base64Layer encoder(stream);
			for (std::size_t offset = 0; offset < length; offset += chunk)
				encoder.write(data.data() + offset, std::min(chunk, length - offset));
			CHECK(encoder.close() == (length > 0));
			CHECK_FALSE(encoder.close());
			CHECK(stream.str() == reference_base64(data));
		}
	}
}

TEST_CASE("Base64 joins VTK length headers and payloads", "[base64]")
{
	const std::vector<double> values = {0, -1, 2.5, 1e20};
	const uint64_t size = values.size() * sizeof(double);
	std::ostringstream stream;
	{
		paraviewo::base64Layer encoder(stream);
		encoder.write(size);
		encoder.write(values.data(), values.size());
	}
	std::string bytes(reinterpret_cast<const char *>(&size), sizeof(size));
	bytes.append(reinterpret_cast<const char *>(values.data()), size);
	CHECK(stream.str() == reference_base64(bytes));
}

TEST_CASE("Base64 reset retains completed groups", "[base64]")
{
	std::ostringstream stream;
	paraviewo::base64Layer encoder(stream);
	encoder.write("foob", 4);
	encoder.reset();
	CHECK(stream.str() == "Zm9v");
	encoder.write("bar", 3);
	CHECK(encoder.close());
	CHECK(stream.str() == "Zm9vYmFy");
	encoder.write("f", 1);
	CHECK(encoder.close());
	CHECK(stream.str() == "Zm9vYmFyZg==");
}
