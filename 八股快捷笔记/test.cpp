#include <iostream>
#include <vector>

long long calAB(int a, int b)
{
    long long result = 0;
    result += ((a + 1) / 2) * ((b + 1) / 2);
    result += (a / 2 + 1) * (b / 2 + 1) - 1;
    return result;
}
long long calN(int n)
{
    long long result = 0;
    long long k = n / 2;
    if (n % 2 == 0)
    {
        result = k * (k - 1);
    }
    else
    {
        result = k * k;
    }
    return result;
}
int main()
{
    using namespace std;
    long long res = 0;
    int N = 0;
    cin >> N;
    vector<long long> num(N, 0);
    for (int i = 0; i < N; i++)
        cin >> num[i];

    for (int i = 0; i < N; i++)
    {
        if (num[i] == 1)
            continue; // 连续1后面统一处理
        int k = i - 1;
        int a = 0;
        int b = 0;
        while (k >= 0 && num[k] == 1)
        {
            a++;
            k--;
        }
        k = i + 1;
        while (k < N && num[k] == 1)
        {
            b++;
            k++;
        }
        res += calAB(a, b);
    }
    int i = 0;
    while (i < N)
    {
        if (num[i] != 1)
        {
            i++;
            continue;
        }
        int count = 1;
        i++;
        while (i < N && num[i] == 1)
        {
            count++;
            i++;
        }
        res += calN(count);
    }

    res += N;
    cout << res;
    return 0;
}