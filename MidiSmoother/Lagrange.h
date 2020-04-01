#include <cstring>
#include <cstdlib>

class Lagrange
{
	static const int N = 100;

private:
	int n, pos;
	double x[N], f[N];

public:
	Lagrange()
	{
		n = pos = 0;
		std::memset(x, 0, sizeof(x));
		std::memset(f, 0, sizeof(f));
	}

	void append(double a, double b)
	{
		x[pos] = a; f[pos++] = b;
		if (pos == N) pos = 0;
		n = n < N ? n + 1 : n;
	}
	double calc(double X)
	{
		double ret = 0;
		for (int i = 0; i < n; i++)
		{
			double up = 1, down = 1;
			for (int j = 0; j < n; j++)
				if (i != j)
				{
					up *= (X - x[j]), down *= (x[i] - x[j]);
				}
			ret += up / down * f[i];
		}
		return ret;
	}
};