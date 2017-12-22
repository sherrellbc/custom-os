__attribute__((__noreturn__))
void abort(void)
{
    //TODO: If compiling as userspace libc, abort with signals
	while(1)
        ;

	__builtin_unreachable();
}
