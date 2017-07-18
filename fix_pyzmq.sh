cd $(pipenv --venv) && sed -i '/_poller_class = Poller/ a\
    def fileno(self):\
        return self.FD\
' $(egrep -lir --include="__init__.py" "class Socket\(_AsyncIO, _future._AsyncSocket\)" .) && cd -