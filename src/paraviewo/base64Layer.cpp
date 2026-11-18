#include "base64Layer.hpp"

namespace paraviewo
{
	namespace
	{
		constexpr char base64Chars[] =
			"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	}

	void base64Layer::flush()
	{
		if (output_size_ == 0)
			return;
		os_.write(output_buffer_.data(), static_cast<std::streamsize>(output_size_));
		output_size_ = 0;
	}

	void base64Layer::append_group(const unsigned char *data, const std::size_t size)
	{
		const unsigned char a = data[0];
		const unsigned char b = size > 1 ? data[1] : 0;
		const unsigned char c = size > 2 ? data[2] : 0;
		output_buffer_[output_size_++] = base64Chars[a >> 2];
		output_buffer_[output_size_++] = base64Chars[((a & 3) << 4) | (b >> 4)];
		output_buffer_[output_size_++] = size > 1 ? base64Chars[((b & 15) << 2) | (c >> 6)] : '=';
		output_buffer_[output_size_++] = size > 2 ? base64Chars[c & 63] : '=';
		if (output_size_ == output_buffer_.size())
			flush();
	}

	base64Layer::base64Layer(std::ostream &os)
		: os_(os), group_(), groupLen_(0), dirty_(false)
	{
	}

	base64Layer::~base64Layer()
	{
		close();
	}

	void base64Layer::write(const char *s, std::streamsize n)
	{
		if (n <= 0)
			return;
		dirty_ = true;

		if (groupLen_ > 0)
		{
			while (groupLen_ < 3 && n > 0)
			{
				group_[groupLen_++] = static_cast<unsigned char>(*s++);
				--n;
			}
			if (groupLen_ < 3)
				return;
			append_group(group_, 3);
			groupLen_ = 0;
		}

		while (n >= 3)
		{
			append_group(reinterpret_cast<const unsigned char *>(s), 3);
			s += 3;
			n -= 3;
		}
		while (n-- > 0)
			group_[groupLen_++] = static_cast<unsigned char>(*s++);
	}

	void base64Layer::reset()
	{
		// Completed groups precede the new sequence. Discard its partial group.
		flush();
		groupLen_ = 0;
		dirty_ = false;
	}

	bool base64Layer::close()
	{
		if (!dirty_)
			return false;
		if (groupLen_ > 0)
			append_group(group_, groupLen_);
		groupLen_ = 0;
		dirty_ = false;
		flush();
		return true;
	}
} // namespace paraviewo
