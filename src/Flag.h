#ifndef FLAG_H
#define FLAG_H

class Flag {
	int m_value;
	Flag(bool);
public:
	Flag(int value):m_value(value) { }
	void Set(int mask) { m_value|=mask; }
	void Unset(int mask) { m_value&=~mask; }
	void Set(int mask, bool set) { if (set) Set(mask); else Unset(mask); }
	bool Get(int mask) const { return (m_value&mask)!=0; }
	int Get() const { return m_value; }
};

inline int FlagIf(int mask, bool set) { return set?mask:0; }

#endif

