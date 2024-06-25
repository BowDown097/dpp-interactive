#pragma once

// this just helps make optional coroutine code more compact
#ifdef DPP_CORO
# include <dpp/coro/task.h>
# define AWAIT(task) co_await task
# define RETURN(res) co_return res
# define RETURN_NO_VALUE co_return
# define TASK(ReturnType) dpp::task<ReturnType>
#else
# define AWAIT(task) task
# define RETURN(res) return res
# define RETURN_NO_VALUE return
# define TASK(ReturnType) ReturnType
#endif
