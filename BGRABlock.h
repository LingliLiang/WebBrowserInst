#pragma once
#ifndef _BGRA_BLOCK_H_
#define _BGRA_BLOCK_H_

class CBGRABlock
{
public:
	typedef struct tagSingleStrip
	{
		LPBYTE p;
		size_t szUnit;
	};

	void FilpMemory(void* pSrc, size_t len, size_t step)
	{
		assert((len%step) == 0);
		assert(step);
		assert(pSrc);
		BYTE* pTemp = new BYTE[step];
		if(!pTemp) return;
		size_t times = len / step / 2;
		BYTE* head = (BYTE*)pSrc,*tail = (BYTE*)pSrc+len-step;
		for(size_t index = 0; index < times; index++)
		{
			::memcpy_s(pTemp,step,head,step);
			::memcpy_s(head,step,tail,step);
			::memcpy_s(tail,step,pTemp,step);
			head += step;
			tail -= step;
		}
		delete pTemp;
	}

	void FilpMemoryHor(void* pSrc,size_t height, size_t width, size_t step)
	{
		assert((width%step) == 0);
		assert(height);
		assert(step);
		assert(pSrc);
		size_t times = height;
		size_t line = width*step;
		BYTE* cur = (BYTE*)pSrc;
		for(size_t index = 0; index < times; index++)
		{
			FilpMemory(cur, line, step);
			cur += line;
		}
	}

	void FilpMemoryVer(void* pSrc,size_t len, size_t width, size_t step)
	{
		FilpMemory(pSrc, len, width*step);
	}

	CBGRABlock(LPBYTE pBGRA, size_t height, size_t width, size_t unit_bytes)
		:m_pHead(pBGRA),m_pTail(NULL),m_Heigh(height),m_Width(width),m_nByteInUnit(unit_bytes)
	{
		m_Rect .left =  m_Rect.top = 0;
		m_Rect.bottom = (LONG)m_Heigh;
		m_Rect.right = (LONG)m_Width;
		m_nAllBytes = (m_Heigh * m_Width * m_nByteInUnit);
		m_pTail = m_pHead + m_nAllBytes;
		InitBlock();
	}

	~CBGRABlock(){}

	ULONG GetRectData(RECT rcRect, LPVOID pByte)
	{
		if(!pByte) return FALSE;
		LPBYTE pDst = (LPBYTE)pByte;
		if(::memcmp(&m_Rect, &rcRect, sizeof(RECT) == 0))
		{
			::memcpy_s(pDst, m_nAllBytes, m_pHead, m_nAllBytes);
			return (ULONG)m_nAllBytes;
		}
		if(::IsRectEmpty(&rcRect)) return FALSE;
		if(rcRect.left < 0 || rcRect.right < 0 || rcRect.top < 0 || rcRect.bottom < 0) return FALSE;
		if(rcRect.bottom > m_Heigh) return FALSE;
		if(rcRect.right > m_Width) return FALSE;
		size_t Heigh = rcRect.bottom - rcRect.top;
		size_t Width = rcRect.right - rcRect.left;
		size_t X = rcRect.left;
		size_t Y = rcRect.top;
		size_t X2 = rcRect.right;
		size_t Y2 = rcRect.bottom;
		size_t Len = Width * m_nByteInUnit;

		if(!m_Block.size()) return FALSE;

		for(size_t index = Y; index < Y2; index++)
		{
			::memcpy_s(pDst, Len, m_Block[index].p + X, Len);
			pDst += Len;
		}

		return (ULONG)(Len*Heigh);
	}

	size_t GetBlockLength() 
	{
		return m_nAllBytes;
	}

	int PasterToBlock(CBGRABlock & other, RECT dstRect)
	{
		if(other.m_nByteInUnit != m_nByteInUnit || other.m_Block.size() != other.m_Heigh) return FALSE;
		if(::IsRectEmpty(&dstRect)) return FALSE;
		if(dstRect.left < 0 || dstRect.right < 0 || dstRect.top < 0 || dstRect.bottom < 0) return FALSE;
		if(dstRect.bottom > other.m_Heigh) return FALSE;
		if(dstRect.right > other.m_Width) return FALSE;
		size_t Heigh = dstRect.bottom - dstRect.top;
		size_t Width = dstRect.right - dstRect.left;
		size_t X = dstRect.left;
		size_t Y = dstRect.top;
		size_t X2 = dstRect.right;
		size_t Y2 = dstRect.bottom;
		size_t Len = Width * m_nByteInUnit;
		size_t index = 0;

		if(m_Block.size() < Heigh) return FALSE;

		for(size_t oindex = Y; oindex < Y2; oindex++)
		{
			::memcpy_s(other.m_Block[oindex].p + X, Len, m_Block[index++].p, Len);
		}
		return TRUE;
	}

private:
	void InitBlock()
	{
		if(m_pHead && m_Heigh && m_Width && m_nByteInUnit)
		{
			auto pByte = m_pHead;
			m_Block.resize(m_Heigh);
			for(auto it = m_Block.begin(); it != m_Block.end(); ++it)
			{
				it->p = pByte;
				it->szUnit = m_Width;
				pByte += m_Width*m_nByteInUnit;
			}
		}
	}

	std::vector<tagSingleStrip> m_Block;

	LPBYTE m_pHead;
	LPBYTE m_pTail;
	size_t m_Heigh;
	size_t m_Width;
	size_t m_nByteInUnit;
	size_t m_nAllBytes;
	RECT m_Rect;
};

#endif  //_BGRA_BLOCK_H_