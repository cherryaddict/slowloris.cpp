# slowloris.cpp
slowloris attack implemented in c++

```
Usage: ./slowloris.exe [-p port] [-t count] [-c count] -u target_url

  Options:

    -p            Port to target. Default: 80
    -t            Number of threads to use. Default: 12
    -c            Number of connections to make per thread. Default: 2000 
    -u            URL to attack

Examples:

./slowloris.exe localhost

./slowloris.exe -p 443 -t 48 -c 5000 localhost
```
