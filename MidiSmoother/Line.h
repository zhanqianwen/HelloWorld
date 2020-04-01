class LinearRegression
{
	static const int N = 100;

private:
	int n, pos;
	double a, b, x[N], f[N];

public:
	LinearRegression()
	{
		n = pos = 0;
		a = b = 0;
		std::memset(x, 0, sizeof(x));
		std::memset(f, 0, sizeof(f));
	}

	void append(double u, double v)
	{
		x[pos] = u; f[pos++] = v;
		if (pos == N) pos = 0;
		n = n < N ? n + 1 : n;
		if (n > 1)
		{
			double _x = 0, _y = 0;
			for (int i = 0; i < n; i++)
				_x += x[i], _y += f[i];
			_x /= (double)n; _y /= (double)n;
			double up = 0, down = 0;
			for (int i = 0; i < n; i++)
			{
				up += (x[i] - _x) * (f[i] - _y);
				down += (x[i] - _x) * (x[i] - _x);
			}
			b = up / down;
			a = _y - b * _x;
		}
	}
	double calc(double X)
	{
		return b * X + a;
	}
};