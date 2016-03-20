// SIP Wrapper for RealTime

%Include stdstring.sip

struct RealTime
{

%TypeHeaderCode
#include <cmath>
#include "base/RealTime.h"
%End

public:
    int sec;
    int nsec;

    int usec() const;
    int msec() const;

    long long toNanoseconds() const;

    RealTime();
    RealTime(int s, int n);

    RealTime(const RealTime &r);
	
    static RealTime fromSeconds(double sec);
    static RealTime fromMilliseconds(int msec);
    static RealTime fromNanoseconds(long long nsec);
    static RealTime fromXsdDuration(std::string xsdd);

    double toDouble() const;

    std::string toString(bool align = false) const;
    static RealTime fromString(std::string);
    std::string toText(bool fixedDp = false) const;
    std::string toFrameText(int fps) const;
    std::string toSecText() const;
    std::string toXsdDuration() const;

    static long realTime2Frame(const RealTime &r, unsigned int sampleRate);
    static RealTime frame2RealTime(long frame, unsigned int sampleRate);

    static const RealTime zeroTime;

    static RealTime abs(const RealTime x);
};
