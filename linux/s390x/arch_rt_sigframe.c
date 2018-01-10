FUNC_GET_RT_SIGFRAME_ADDR
{
	return tcp->currpers == 1 ? *s390_frame_ptr : *s390x_frame_ptr;
}
