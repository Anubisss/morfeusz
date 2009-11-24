/*
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
 *
 * Copyright (C) 2008 Trinity <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef _BYTEBUFFER_H
#define _BYTEBUFFER_H

#include "Common.h"
#include "Util.h"

#include <map>
#include <list>

class ByteBuffer
{
    public:
        const static size_t DEFAULT_SIZE = 0x1000;

        // constructor
        ByteBuffer(): _rpos(0), _wpos(0)
        {
            _storage.reserve(DEFAULT_SIZE);
        }
        // constructor
	ByteBuffer(size_t res): _rpos(0), _wpos(0)
        {
            _storage.reserve(res);
        }
        // copy constructor
        ByteBuffer(const ByteBuffer &buf): _rpos(buf._rpos), _wpos(buf._wpos), _storage(buf._storage) { }

        void clear()
        {
            _storage.clear();
            _rpos = _wpos = 0;
        }

        template <typename T> void append(T value)
        {
            Utils::EndianConvert(value);
            append((uint8 *)&value, sizeof(value));
        }

        template <typename T> void put(size_t pos,T value)
        {
            Utils::EndianConvert(value);
            put(pos,(uint8 *)&value,sizeof(value));
        }

        ByteBuffer &operator<<(uint8 value)
        {
            append<uint8>(value);
            return *this;
        }
        ByteBuffer &operator<<(uint16 value)
        {
            append<uint16>(value);
            return *this;
        }
        ByteBuffer &operator<<(uint32 value)
        {
            append<uint32>(value);
            return *this;
        }
        ByteBuffer &operator<<(uint64 value)
        {
            append<uint64>(value);
            return *this;
        }

        // signed as in 2e complement
        ByteBuffer &operator<<(int8 value)
        {
            append<int8>(value);
            return *this;
        }
        ByteBuffer &operator<<(int16 value)
        {
            append<int16>(value);
            return *this;
        }
        ByteBuffer &operator<<(int32 value)
        {
            append<int32>(value);
            return *this;
        }
        ByteBuffer &operator<<(int64 value)
        {
            append<int64>(value);
            return *this;
        }

        // floating points
        ByteBuffer &operator<<(float value)
        {
            append<float>(value);
            return *this;
        }
        ByteBuffer &operator<<(double value)
        {
            append<double>(value);
            return *this;
        }
        ByteBuffer &operator<<(const std::string &value)
        {
            append((uint8 const *)value.c_str(), value.length());
            append((uint8)0);
            return *this;
        }
        ByteBuffer &operator<<(const char *str)
        {
            append((uint8 const *)str, str ? strlen(str) : 0);
            append((uint8)0);
            return *this;
        }

        ByteBuffer &operator>>(bool &value)
        {
            value = read<char>() > 0 ? true : false;
            return *this;
        }

        ByteBuffer &operator>>(uint8 &value)
        {
            value = read<uint8>();
            return *this;
        }
        ByteBuffer &operator>>(uint16 &value)
        {
            value = read<uint16>();
            return *this;
        }
        ByteBuffer &operator>>(uint32 &value)
        {
            value = read<uint32>();
            return *this;
        }
        ByteBuffer &operator>>(uint64 &value)
        {
            value = read<uint64>();
            return *this;
        }

        //signed as in 2e complement
        ByteBuffer &operator>>(int8 &value)
        {
            value = read<int8>();
            return *this;
        }
        ByteBuffer &operator>>(int16 &value)
        {
            value = read<int16>();
            return *this;
        }
        ByteBuffer &operator>>(int32 &value)
        {
            value = read<int32>();
            return *this;
        }
        ByteBuffer &operator>>(int64 &value)
        {
            value = read<int64>();
            return *this;
        }

        ByteBuffer &operator>>(float &value)
        {
            value = read<float>();
            return *this;
        }
        ByteBuffer &operator>>(double &value)
        {
            value = read<double>();
            return *this;
        }
        ByteBuffer &operator>>(std::string& value)
        {
            value.clear();
            while (rpos() < size())                         // prevent crash at wrong string format in packet
            {
                char c=read<char>();
                if (c==0)
                    break;
                value+=c;
            }
            return *this;
        }

        uint8 operator[](size_t pos) const
        {
            return read<uint8>(pos);
        }

        size_t rpos() const { return _rpos; }

        size_t rpos(size_t rpos_)
        {
            _rpos = rpos_;
            return _rpos;
        };

        size_t wpos() const { return _wpos; }

        size_t wpos(size_t wpos_)
        {
            _wpos = wpos_;
            return _wpos;
        }

        template <typename T> T read()
        {
            T r=read<T>(_rpos);
            _rpos += sizeof(T);
            return r;
        };
        template <typename T> T read(size_t pos) const
        {
            ASSERT(pos + sizeof(T) <= size() || PrintPosError(false,pos,sizeof(T)));
            T val = *((T const*)&_storage[pos]);
            Utils::EndianConvert(val);
            return val;
        }

        void read(uint8 *dest, size_t len)
        {
            ASSERT(_rpos  + len  <= size() || PrintPosError(false,_rpos,len));
            memcpy(dest, &_storage[_rpos], len);
            _rpos += len;
        }

        const uint8 *contents() const { return &_storage[0]; }

        size_t size() const { return _storage.size(); }
        bool empty() const { return _storage.empty(); }

        void resize(size_t newsize)
        {
            _storage.resize(newsize);
            _rpos = 0;
            _wpos = size();
        };
        void reserve(size_t ressize)
        {
            if (ressize > size()) _storage.reserve(ressize);
        };

        void append(const std::string& str)
        {
            append((uint8 const*)str.c_str(),str.size() + 1);
        }
        void append(const char *src, size_t cnt)
        {
            return append((const uint8 *)src, cnt);
        }
        template<class T>
        void append(const T *src, size_t cnt)
        {
            return append((const uint8 *)src, cnt*sizeof(T));
        }
        void append(const uint8 *src, size_t cnt)
        {
            if (!cnt) return;

            ASSERT(size() < 10000000);

            if (_storage.size() < _wpos + cnt)
                _storage.resize(_wpos + cnt);
            memcpy(&_storage[_wpos], src, cnt);
            _wpos += cnt;
        }
        void append(const ByteBuffer& buffer)
        {
            if(buffer.size()) append(buffer.contents(),buffer.size());
        }

        void appendPackGUID(uint64 guid)
        {
            size_t mask_position = wpos();
            *this << uint8(0);
            for(uint8 i = 0; i < 8; i++)
            {
                if(guid & 0xFF)
                {
                    _storage[mask_position] |= uint8(1<<i);
                    *this << uint8(guid & 0xFF);
                }

                guid >>= 8;
            }
        }

        void put(size_t pos, const uint8 *src, size_t cnt)
        {
            ASSERT(pos + cnt <= size() || PrintPosError(true,pos,cnt));
            memcpy(&_storage[pos], src, cnt);
        }
        void print_storage() const
        {

        }

        void textlike() const
        {
	  /*            if(!sLog.IsOutDebug())                          // optimize disabled debug output
                return;

            sLog.outDebug("STORAGE_SIZE: %u", size() );
            for(uint32 i = 0; i < size(); i++)
                sLog.outDebugInLine("%c", read<uint8>(i) );
		sLog.outDebug(" "); */
        }

        void hexlike() const
        {
	  /*           if(!sLog.IsOutDebug())                          // optimize disabled debug output
                return;

            uint32 j = 1, k = 1;
            sLog.outDebug("STORAGE_SIZE: %u", size() );

            if(sLog.IsIncludeTime())
                sLog.outDebugInLine("         ");

            for(uint32 i = 0; i < size(); i++)
            {
                if ((i == (j*8)) && ((i != (k*16))))
                {
                    if (read<uint8>(i) < 0x0F)
                    {
                        sLog.outDebugInLine("| 0%X ", read<uint8>(i) );
                    }
                    else
                    {
                        sLog.outDebugInLine("| %X ", read<uint8>(i) );
                    }
                    ++j;
                }
                else if (i == (k*16))
                {
                    if (read<uint8>(i) < 0x0F)
                    {
                        sLog.outDebugInLine("\n");
                        if(sLog.IsIncludeTime())
                            sLog.outDebugInLine("         ");

                        sLog.outDebugInLine("0%X ", read<uint8>(i) );
                    }
                    else
                    {
                        sLog.outDebugInLine("\n");
                        if(sLog.IsIncludeTime())
                            sLog.outDebugInLine("         ");

                        sLog.outDebugInLine("%X ", read<uint8>(i) );
                    }

                    ++k;
                    ++j;
                }
                else
                {
                    if (read<uint8>(i) < 0x0F)
                    {
                        sLog.outDebugInLine("0%X ", read<uint8>(i) );
                    }
                    else
                    {
                        sLog.outDebugInLine("%X ", read<uint8>(i) );
                    }
                }
            }
            sLog.outDebugInLine("\n");
	  */}

    protected:
        bool PrintPosError(bool add, size_t pos, size_t esize) const
        {
	  //sLog.outError("ERROR: Attempt %s in ByteBuffer (pos: %u size: %u) value with size: %u",(add ? "put" : "get"),pos, size(), esize);

            // assert must fail after function call
            return false;
        }

        size_t _rpos, _wpos;
        std::vector<uint8> _storage;
};

/**
 * @brief Packet recieved by server.
 * @detail This class was created only to allow
 *         for easy acces into opcode and size of recv'd data.
 */
class ClientPkt : public ByteBuffer
{
 public:
 ClientPkt(size_t siz): ByteBuffer(siz){}
 ClientPkt():ByteBuffer(){}
  
  /**
   * @brief Peek opcode, without moving internal read position
   */
  uint32 PeekOpcode()
  {
    size_t tmp;
    uint32 ret;
    tmp = _rpos;
    _rpos = 2;
    ret = read<uint32>();
    _rpos = tmp;
    return ret;
  }

  /**
   * @brief Analogous to PeekOpcode, but for size.
   */
  uint16 PeekSize()
  {
    size_t tmp;
    uint16 ret;
    tmp = _rpos;
    _rpos = 0;
    ret = read<uint16>();
    _rpos = tmp;
    return ret;
  }

};

/**
 * @brief Packet sent by server.
 */
class ServerPkt : public ByteBuffer
{
 public:

  /**
   * @brief We add +4 so when passing length of 
   *        data do not add header length.
   */
 ServerPkt(size_t siz) : ByteBuffer(siz + 4){_wpos = 4;}

  /**
   * @brief By default, reserve only header.
   */
 ServerPkt() : ByteBuffer(4){_wpos = 4;}
  void SetOpcode(uint16 op)
    {
      size_t tmp;
      tmp = _wpos;
      _wpos = 2;
      Utils::EndianConvert<uint16>(op);
      append<uint16>(op);
      _wpos = tmp;

    }

  /**
   * @brief When setting size, we are ONLY talking 
   *        about size of data, size of header is 
   *        added to size of payload.
   */
  void SetSize(uint16 size)
    {
      size_t tmp;
      size += 2;
      tmp = _wpos;
      _wpos = 0;
      Utils::EndianConvertReverse<uint16>(size);
      append<uint16>(size);
      _wpos = tmp;
    }
};

template <typename T> ByteBuffer &operator<<(ByteBuffer &b, std::vector<T> v)
{
    b << (uint32)v.size();
    for (typename std::vector<T>::iterator i = v.begin(); i != v.end(); i++)
    {
        b << *i;
    }
    return b;
}

template <typename T> ByteBuffer &operator>>(ByteBuffer &b, std::vector<T> &v)
{
    uint32 vsize;
    b >> vsize;
    v.clear();
    while(vsize--)
    {
        T t;
        b >> t;
        v.push_back(t);
    }
    return b;
}

template <typename T> ByteBuffer &operator<<(ByteBuffer &b, std::list<T> v)
{
    b << (uint32)v.size();
    for (typename std::list<T>::iterator i = v.begin(); i != v.end(); i++)
    {
        b << *i;
    }
    return b;
}

template <typename T> ByteBuffer &operator>>(ByteBuffer &b, std::list<T> &v)
{
    uint32 vsize;
    b >> vsize;
    v.clear();
    while(vsize--)
    {
        T t;
        b >> t;
        v.push_back(t);
    }
    return b;
}

template <typename K, typename V> ByteBuffer &operator<<(ByteBuffer &b, std::map<K, V> &m)
{
    b << (uint32)m.size();
    for (typename std::map<K, V>::iterator i = m.begin(); i != m.end(); i++)
    {
        b << i->first << i->second;
    }
    return b;
}

template <typename K, typename V> ByteBuffer &operator>>(ByteBuffer &b, std::map<K, V> &m)
{
    uint32 msize;
    b >> msize;
    m.clear();
    while(msize--)
    {
        K k;
        V v;
        b >> k >> v;
        m.insert(make_pair(k, v));
    }
    return b;
}
#endif

