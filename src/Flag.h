#ifndef FLAG_H
#define FLAG_H

class Flag {
	int value;
	Flag(bool);
public:
	Flag(int _value):value(_value) { }
	void Set(int mask) { value|=mask; }
	void Unset(int mask) { value&=~mask; }
	void Set(int mask, bool set) { if (set) Set(mask); else Unset(mask); }
	bool Get(int mask) const { return (value&mask)!=0; }
	int Get() const { return value; }
};

inline int FlagIf(int mask, bool set) { return set?mask:0; }

#endif

