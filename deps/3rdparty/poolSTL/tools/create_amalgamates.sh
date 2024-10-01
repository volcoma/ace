#!/usr/bin/env bash

python amalgamate.py ../include/poolstl/poolstl.hpp --header amalgamate_header.hpp -o poolstl.hpp

# create a version that excludes the thread pool
python amalgamate.py ../include/poolstl/poolstl.hpp --header amalgamate_header.hpp -o poolstl_nopool.hpp -e ../include/poolstl/internal/task_thread_pool.hpp

