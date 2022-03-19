#pragma once

#include <cstdint>
#include <streambuf>
#include <stdio.h>
#include <string>
#include <limits>
#include <ios>
#include <sstream>
#include <type_traits>

namespace arduino {

	/**
	 * Adapts a OSTREAM for printing single characters at a time.
	 *
	 * This version only uses the single-byte 'write' function of Print
	 * to write to the OSTREAM.
	 */
	template <class OSTREAM>
	class Print_stdostream_put : public Print {
	public:
		Print_stdostream_put(OSTREAM& os) : _ostream(os) {}
		OSTREAM& ostream() { return _ostream; }

		/// write a single byte
		virtual size_t write(const uint8_t c) override {
			_ostream.put(static_cast<const char>(c));
			return _ostream.good() ? 1 : 0;
		}
		using Print::write;
		using Print::print;
		using Print::println;
		using Print::flush;

		/// number of bytes available in write buffer.
		virtual int availableForWrite() override {
			return std::numeric_limits<int>::max();
		}

		template<typename T>
		OSTREAM& operator<< (T t) { return ostream() << t; }
	protected:
		OSTREAM& _ostream;
	};

	/**
	 * Adapts a OSTREAM for printing.
	 *
	 * This version overrides both the single-byte 'write' and the
	 * multi-byte 'write' functions of Print to write to the OSTREAM.
	 */
	template <class OSTREAM>
	class Print_stdostream : public Print {
	public:
		Print_stdostream(OSTREAM& os) : _ostream(os) {}
		OSTREAM& ostream() { return _ostream; }

		/// write a single byte
		virtual size_t write(const uint8_t c) override {
			_ostream.put(static_cast<const char>(c));
			return _ostream.good() ? 1 : 0;
		}
		/// write multible bytes
		virtual size_t write(const uint8_t* str, size_t n) override {
			auto first = _ostream.tellp();
			_ostream.write(reinterpret_cast<const char*>(str), n);
			return _ostream.good() ? n : _ostream.tellp() - first;
		}
		using Print::write;
		using Print::print;
		using Print::println;
		using Print::flush;

		/// number of bytes available in write buffer.
		virtual int availableForWrite() override {
			return std::numeric_limits<int>::max();
		}

		template<typename T>
		OSTREAM& operator<< (T t) { return ostream() << t; }
	protected:
		OSTREAM& _ostream;
	};

	/**
	 * Adapts a std::ostringstream for printing to strings.
	 *
	 * This version keeps an internal stringstream/stringbuf
	 * as its stream. Access the current contents with @see str().
	 * Clear the buffer with @see clear().
	 */
	class Print_stdstring : public Print_stdostream<std::ostringstream> {
	public:
		Print_stdstring(const std::string str)
			: _oss(str, std::ios_base::out | std::ios_base::app), Print_stdostream(_oss) {
			// NOTE: open in append mode so we don't overwrite the intiial value
		}
		Print_stdstring()
			: _oss(std::ios_base::out | std::ios_base::app), Print_stdostream(_oss) {
			// NOTE: open in append mode so we don't overwrite current contents
		}
		std::string str() const { return _oss.str(); }
		void str(const std::string s) { _oss.str(s); }
		void clear() { _oss.str(""); _oss.clear(); }

		/// number of bytes available in write buffer. 
		/// For stringstream it will return 0 just before reallocating more buffer space
		virtual int availableForWrite() override {
			return str().capacity() - str().length();
		}
	protected:
		std::ostringstream _oss;
	};


} // namespace arduino