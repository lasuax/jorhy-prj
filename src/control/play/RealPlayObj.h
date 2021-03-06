#ifndef __REALPLAYOBJ_H_
#define __REALPLAYOBJ_H_
#include "j_includes.h"
#include "x_adapter_manager.h"

class CRealPlayObj : public J_PlayerObj
{
public:
	CRealPlayObj(int nStreamId, const char *pPlayerType, int nStreamType, const char *pResid);
	~CRealPlayObj();

public:
	///J_PlayerObj
	virtual j_result_t PlayMedia(j_wnd_t hWnd, j_int32_t nDevid);
	virtual j_result_t StopMedia(j_int32_t nDevid);
	virtual j_result_t ProcessMedia();
	virtual j_result_t AspectRatio(j_int32_t nWidth, j_int32_t nHeight);

private:
	//ÊµÊ±ÊÓÆµ
	j_result_t StartVideo(j_int32_t nDevid);
	j_result_t StopVideo(j_int32_t nDevid);

private:
	j_socket_t m_nSocket;
	j_int32_t m_nStreamType;
	CRingBuffer *m_pRingBuffer;
	j_boolean_t m_bStart;
	j_char_t *m_pDataBuff;
	j_char_t *m_pConvetBuff;

	j_int64_t m_nextFrameTime;
	j_int64_t m_lastFrameTime;
	j_uint32_t m_lastFrameNum;

	j_string_t m_resid;
	j_string_t m_playerType;
	J_StreamHeader m_streamHeader;
};

#endif //~__REALPLAYOBJ_H_