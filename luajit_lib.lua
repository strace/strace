-- This "chunk" of code is loaded and run before the script is.
--
-- To quote https://www.lua.org/manual/5.1/manual.html#2.4.1,
-- "Lua handles a chunk as the body of an anonymous function with a variable
--  number of arguments (see §2.5.9). As such, chunks can define local
--  variables, receive arguments, and return values."
--
-- Thanks to Lua's support for closures, all the local variables defined here
-- will not leak to another chunks (i.e., the script), but all the functions
-- defined here can still access them.
--
-- strace calls this chunk with a single argument: a table with data that should
-- not be exposed to the script, but is needed for some API functions defined
-- here.
--
-- strace expects this chunk to return another function that will be run after
-- the script returns.
--
-- Arguments passed to this chunk are accessible through the "..." vararg
-- expression. The following line uses Lua's "adjust" assignment semantics to
-- assign the first argument to a local variable "priv".
local priv = ...

local ffi = require(priv.ffilibname)
ffi.cdef[[
int strcmp(const char *, const char *);
]]
local bit = require(priv.bitlibname)

local at_exit_cb
local entry_cbs, exit_cbs = {}, {}
for p = 0, strace.npersonalities - 1 do
	entry_cbs[p] = {}
	exit_cbs[p] = {}
end

local tcp

local nullptr = ffi.NULL
pcall(function()
	nullptr = ffi.C.NULL
end)

local function chain(f, g)
	if f == nil then
		return g
	end
	return function(...)
		f(...)
		g(...)
	end
end

local function register_hook(scno, pers, on_entry, on_exit, cb)
	assert(not not strace.C.monitor(scno, pers, on_entry, on_exit),
		'unexpected strace.C.monitor failure')
	scno, pers = tonumber(scno), tonumber(pers)
	if on_entry then
		entry_cbs[pers][scno] = chain(entry_cbs[pers][scno], cb)
	end
	if on_exit then
		exit_cbs[pers][scno] = chain(exit_cbs[pers][scno], cb)
	end
end

local priv_next_sc = priv.next_sc

function strace.next_sc()
	local ptr = priv_next_sc()
	tcp = ptr ~= nullptr and ptr or nil
	return tcp
end

function strace.entering()
	return bit.band(tcp.flags, priv.tcb_insyscall) == 0
end

function strace.exiting()
	return bit.band(tcp.flags, priv.tcb_insyscall) ~= 0
end

local function alter_trace_opt(flagbit, ...)
	assert(strace.entering(),
		'altering tracing options must be done on syscall entry')
	-- i.e., if ... is empty, or the first element of ... is true
	if select('#', ...) == 0 or select(1, ...) then
		tcp.qual_flg = bit.bor(tcp.qual_flg, flagbit)
	else
		tcp.qual_flg = bit.band(tcp.qual_flg, bit.bnot(flagbit))
	end
end
function strace.trace(...)	alter_trace_opt(priv.qual_trace, ...)	end
function strace.abbrev(...)	alter_trace_opt(priv.qual_abbrev, ...)	end
function strace.verbose(...)	alter_trace_opt(priv.qual_verbose, ...)	end
function strace.raw(...)	alter_trace_opt(priv.qual_raw, ...)	end

local ulong_t = ffi.typeof('unsigned long')
local kulong_t = ffi.typeof('kernel_ulong_t')

function strace.ptr_to_kulong(ptr)
	return ffi.cast(kulong_t, ffi.cast(ulong_t, ptr))
end

function strace.at_exit(f)
	at_exit_cb = chain(at_exit_cb, f)
end

local function validate_pers(pers)
	assert(pers >= 0 and pers < strace.npersonalities,
		'invalid personality number')
end

function strace.get_sc_name(scno, pers)
	validate_pers(pers)
	if scno < 0 or scno >= strace.C.nsysent_vec[pers] then
		return nil
	end
	local s = strace.C.sysent_vec[pers][scno].sys_name
	return s ~= nullptr and ffi.string(s) or nil
end

function strace.get_sc_flags(scno, pers)
	validate_pers(pers)
	if scno < 0 or scno >= strace.C.nsysent_vec[pers] then
		return nil
	end
	local entry = strace.C.sysent_vec[pers][scno]
	return entry.sys_name ~= nullptr and entry.sys_flags or nil
end

function strace.get_sig_name(signo, pers)
	validate_pers(pers)
	if signo < 0 or signo >= strace.C.nsignalent_vec[pers] then
		return nil
	end
	local s = strace.C.signalent_vec[pers][signo]
	return s ~= nullptr and ffi.string(s) or nil
end

function strace.get_err_name(err, pers)
	validate_pers(pers)
	if err < 0 or err > strace.C.nerrnoent_vec[pers] then
		return nil
	end
	local s = strace.C.errnoent_vec[pers][err]
	return s ~= nullptr and ffi.string(s) or nil
end

local uint_t = ffi.typeof('unsigned int')

function strace.get_ioctl_name(code, pers)
	validate_pers(pers)
	-- we could have provided a definition for stdlib's bsearch() and used
	-- it, but LuaJIT's FFI manual says generated callbacks are a limited
	-- resource and also slow. So implement binary search ourselves.
	local lb, rb = uint_t(0), strace.C.nioctlent_vec[pers]
	if rb == 0 then
		return nil
	end
	local arr = strace.C.ioctlent_vec[pers]
	while rb - lb > 1 do
		local mid = lb + (rb - lb) / 2
		if arr[mid].code <= code then
			lb = mid
		else
			rb = mid
		end
	end
	return arr[lb].code == code and ffi.string(arr[lb].symbol) or nil
end

local const_cstr_t = ffi.typeof('const char *')

local function validate_cstr_initializer(s)
	-- ffi.istype() ignores constness, so the following accepts a cdata
	-- (non-const) char * as well.
	assert(type(s) == 'string' or ffi.istype(const_cstr_t, s),
		'expected either a Lua string or a cdata C string')
end

function strace.get_class_flagbit(clsname)
	validate_cstr_initializer(clsname)
	local cstr = const_cstr_t(clsname)
	local ptr = strace.C.syscall_classes
	while ptr.name ~= nullptr do
		if ffi.C.strcmp(ptr.name, cstr) == 0 then
			return ptr.value
		end
		ptr = ptr + 1
	end
	return nil
end

function strace.get_ioctl_code(name, pers)
	validate_cstr_initializer(name)
	validate_pers(pers)
	local cstr = const_cstr_t(name)
	local arr = strace.C.ioctlent_vec[pers]
	for i = 0, tonumber(strace.C.nioctlent_vec[pers]) - 1 do
		if ffi.C.strcmp(arr[i].symbol, cstr) == 0 then
			return arr[i].code
		end
	end
	return nil
end

function strace.get_scno(scname, pers)
	validate_cstr_initializer(scname)
	validate_pers(pers)
	local cstr = const_cstr_t(scname)
	local arr = strace.C.sysent_vec[pers]
	for i = 0, tonumber(strace.C.nsysent_vec[pers]) - 1 do
		local s = arr[i].sys_name
		if s ~= nullptr and ffi.C.strcmp(s, cstr) == 0 then
			return i
		end
	end
	return nil
end

function strace.get_signo(signame, pers)
	validate_cstr_initializer(signame)
	validate_pers(pers)
	local cstr = const_cstr_t(signame)
	local arr = strace.C.signalent_vec[pers]
	for i = 0, tonumber(strace.C.nsignalent_vec[pers]) - 1 do
		local s = arr[i]
		if s ~= nullptr and ffi.C.strcmp(s, cstr) == 0 then
			return i
		end
	end
	return nil
end

function strace.get_errno(errname, pers)
	validate_cstr_initializer(errname)
	validate_pers(pers)
	local cstr = const_cstr_t(errname)
	local arr = strace.C.errnoent_vec[pers]
	for i = 0, tonumber(strace.C.nerrnoent_vec[pers]) - 1 do
		local s = arr[i]
		if s ~= nullptr and ffi.C.strcmp(s, cstr) == 0 then
			return i
		end
	end
	return nil
end

function strace.inject_signal(sig)
	local signo = tonumber(sig)
	if signo == nil then
		signo = strace.get_signo(sig, tcp.currpers)
		if signo == nil then
			return false, 'signal not found'
		end
	end
	if not strace.C.inject_signo(signo) then
		return false, 'cannot inject signal'
	end
	return true
end

function strace.inject_error(err)
	local errno = tonumber(err)
	if errno == nil then
		errno = strace.get_errno(err, tcp.currpers)
		if errno == nil then
			return false, 'error not found'
		end
	end
	if errno <= 0 or not strace.C.inject_retval(-errno) then
		return false, 'cannot inject error'
	end
	return true
end

function strace.inject_retval(val)
	if not strace.C.inject_retval(val) then
		return false, 'cannot inject return value'
	end
	return true
end

local ptr_size = ffi.sizeof(const_cstr_t)

-- Calls f(addr, ffi.sizeof(obj), P), where P is a pointer to obj (or to its
-- copy; see below), and f is a cdata C function that takes a (possibly const)
-- void * as the last argument.
-- Returns the result of that call.
-- This function may decide to make a copy of obj, and, in this case, returns
-- that copy as an additional return value.
local function call_ufunc_on(f, addr, obj)
	local n = ffi.sizeof(obj)
	if n ~= ptr_size then
		-- either a structure/union, an array, a scalar, or a VLA
		local is_ok, ret = pcall(f, addr, n, obj)
		if is_ok then
			-- not a scalar
			return ret
		end
	end
	-- either a structure/union, an array, a scalar, a VLA, or a pointer
	local obj_t = ffi.typeof(obj)
	-- FFI templating fails for VLAs
	local is_ok, arr_t = pcall(ffi.typeof, '$ [1]', obj_t)
	if is_ok then
		-- not a VLA
		local arr = arr_t(obj)
		local ret = f(addr, n, arr)
		return ret, obj_t(arr[0])
	end
	-- a VLA
	return f(addr, n, obj)
end

function strace.read_obj(addr, ct, ...)
	local obj = ffi.new(ct, ...)
	local status, copy = call_ufunc_on(strace.C.umove, addr, obj)
	return status == 0 and (copy or obj) or nil
end

function strace.read_str(addr, maxsz, bufsz)
	-- convert it to Lua number to prevent underflows
	maxsz = tonumber(maxsz) or 4 * 1024 * 1024
	bufsz = bufsz or 1024
	local t = {}
	local buf = ffi.new('char [?]', bufsz)
	while true do
		local r = strace.C.umove_str(addr, bufsz, buf)
		if r < 0 then
			return nil, 'read error'
		elseif r == 0 then
			maxsz = maxsz - bufsz
			if maxsz < 0 then
				return nil, 'string is too long'
			end
			t[#t + 1] = ffi.string(buf, bufsz)
			addr = addr + bufsz
		else
			local s = ffi.string(buf)
			if #s > maxsz then
				return nil, 'string is too long'
			end
			t[#t + 1] = s
			return table.concat(t)
		end
	end
end

function strace.read_path(addr)
	return strace.read_str(addr, strace.path_max - 1, strace.path_max)
end

function strace.write_obj(addr, obj)
	return call_ufunc_on(strace.C.upoke, addr, obj) == 0
end

local function parse_when(when)
	if type(when) == 'table' then
		return unpack(when)
	elseif when == 'entering' then
		return true, false
	elseif when == 'exiting' then
		return false, true
	elseif when == 'both' then
		return true, true
	else
		error('unknown "when" value')
	end
end

local function reduce_or(f, args, ...)
	local ret = false
	for _, arg in ipairs(args) do
		if f(arg, ...) then
			ret = true
		end
	end
	return ret
end

function strace.hook(scname, when, cb)
	local on_entry, on_exit = parse_when(when)
	if type(scname) == 'table' then
		return reduce_or(strace.hook, scname, {on_entry, on_exit}, cb)
	end
	local found = false
	for p = 0, strace.npersonalities - 1 do
		local scno = strace.get_scno(scname, p)
		if scno ~= nil then
			register_hook(scno, p, on_entry, on_exit, cb)
			found = true
		end
	end
	return found
end

function strace.hook_class(clsname, when, cb)
	local on_entry, on_exit = parse_when(when)
	if type(clsname) == 'table' then
		return reduce_or(strace.hook_class, clsname,
			{on_entry, on_exit}, cb)
	end
	local flag = strace.get_class_flagbit(clsname)
	if flag == nil then
		return false
	end
	for p = 0, strace.npersonalities - 1 do
		local arr = strace.C.sysent_vec[p]
		for i = 0, tonumber(strace.C.nsysent_vec[p]) - 1 do
			if bit.band(arr[i].sys_flags, flag) ~= 0 then
				register_hook(i, p, on_entry, on_exit, cb)
			end
		end
	end
	return true
end

function strace.hook_scno(scno, pers, when, cb)
	validate_pers(pers)
	local on_entry, on_exit = parse_when(when)
	if type(scno) == 'table' then
		return reduce_or(strace.hook_scno, scno, pers,
			{on_entry, on_exit}, cb)
	end
	if scno < 0 then
		return false
	end
	register_hook(scno, pers, on_entry, on_exit, cb)
	return true
end

function strace.path_match(set)
	if type(set) ~= 'table' then
		set = {set}
	end
	for _, elem in pairs(set) do
		validate_cstr_initializer(elem)
	end
	local nset = #set
	return not not strace.C.path_match(
		ffi.new('const char *[?]', nset, set), nset)
end

function print(...)
	local file = io.stderr
	local sep = ''
	for i = 1, select('#', ...) do
		file:write(sep .. tostring(select(i, ...)))
		sep = '\t'
	end
	file:write('\n')
end

return function()
	local next_sc = strace.next_sc
	local entering = strace.entering
	while next_sc() ~= nil do
		local cb = (entering() and entry_cbs or exit_cbs)
			[tonumber(tcp.currpers)][tonumber(tcp.scno)]
		if cb ~= nil then
			cb(tcp)
		end
	end
	if at_exit_cb ~= nil then
		at_exit_cb()
	end
end
