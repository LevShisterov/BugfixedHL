// Martin Webrant (BulliT)

#ifndef __AGHUDVOTE_H__
#define __AGHUDVOTE_H__

class AgHudVote : public CHudBase
{
public:
	AgHudVote() : m_flTurnoff(0), m_iVoteStatus(0), m_iFor(0), m_iAgainst(0), m_iUndecided(0), m_byVoteStatus(0) { }

	int Init(void) override;
	int VidInit(void) override;
	int Draw(float flTime) override;
	void Reset(void) override;

	int MsgFunc_Vote(const char *pszName, int iSize, void *pbuf);

private:
	enum VoteStatus { NotRunning = 0, Called = 1, Accepted = 2, Denied = 3, };

	float m_flTurnoff;
	int m_iVoteStatus;
	int m_iFor;
	int m_iAgainst;
	int m_iUndecided;
	char m_byVoteStatus;
	char m_szVote[32];
	char m_szValue[32];
	char m_szCalled[32];
};

#endif // __AGHUDVOTE_H__
