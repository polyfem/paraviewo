#pragma once

#include <iostream>
#include <cstdint>
#include <array>

namespace paraviewo
{
	class base64Layer
	{
	private:
		//- The output stream for the layer
		std::ostream &os_;

		//- Buffer of characters to encode
		unsigned char group_[3];

		//- Current length of the encode buffer
		unsigned char groupLen_;

		//- Track if anything has been encoded.
		bool dirty_;

		std::array<char, 64 * 1024> output_buffer_;
		std::size_t output_size_ = 0;

		base64Layer(const base64Layer &) = delete;
		void operator=(const base64Layer &) = delete;

		void append_group(const unsigned char *data, std::size_t size);
		void flush();

	public:
		base64Layer(std::ostream &os);
		~base64Layer();

		//- Encode the character sequence, writing when possible.
		void write(const char *s, std::streamsize n);

		inline void write(const uint64_t v) { write(reinterpret_cast<const char *>(&v), sizeof(uint64_t)); }
		inline void write(const int64_t v) { write(reinterpret_cast<const char *>(&v), sizeof(int64_t)); }
		inline void write(const int8_t v) { write(reinterpret_cast<const char *>(&v), sizeof(int8_t)); }
		inline void write(const uint8_t v) { write(reinterpret_cast<const char *>(&v), sizeof(uint8_t)); }

		inline void write(const int64_t *v, const int n) { write(reinterpret_cast<const char *>(v), n * sizeof(int64_t)); }
		inline void write(const double *v, const int n) { write(reinterpret_cast<const char *>(v), n * sizeof(double)); }
		inline void write(const float *v, const int n) { write(reinterpret_cast<const char *>(v), n * sizeof(float)); }

		//- Restart a new encoding sequence.
		void reset();

		//- End the encoding sequence, padding the final characters with '='.
		//  Return false if no encoding was actually performed.
		bool close();
	};
} // namespace paraviewo
