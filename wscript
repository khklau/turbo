from waflib.extras.preparation import PreparationContext
from waflib.extras.build_status import BuildStatus

def options(optCtx):
    optCtx.recurse('env')
    optCtx.recurse('src')

def prepare(prepCtx):
    prepCtx.recurse('env')
    prepCtx.recurse('src')

def configure(confCtx):
    confCtx.recurse('env')
    confCtx.recurse('src')
    
def build(buildCtx):
    status = BuildStatus.init(buildCtx.path.abspath())
    buildCtx.recurse('src')
    status.setSuccess()

def install(installCtx):
    installCtx.recurse('src')
