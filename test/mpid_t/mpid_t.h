struct mpid_t
{
    private:
        pid_t pid_;

    public:

        pid_t operator* () { return pid_; }

        mpid_t(pid_t pid)
            :pid_(pid)
        {
            if ( pid < 0) 
                throw runtime_error("trying to construct mpid_t from a bad pid")
        }

        mpid_t(mpid_t &&other) 
            :pid_(other.pid_)
        {
            other.pid = 0;
        }
}
