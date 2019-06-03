# Todo: privilege checking
# Todo: filter input(Flask WTF)
# Todo: error handling
# Todo: more friendly message alert

from datetime import timedelta
from time import time

from flask import Flask, g
from flask_wtf.csrf import CSRFProtect, CSRFError
from blueprint.utility import logger
from blueprint.UserSystem import UserSystem
from blueprint.TicketSystem import TicketSystem
from blueprint.TrainSystem import TrainSystem
from blueprint.AdminSystem import AdminSystem

from client import init

app = Flask(__name__)
app.config.update(SECRET_KEY='seCreT Key',
                  PERMANENT_SESSION_LIFETIME=timedelta(days=7)
                  )
#CSRFProtect(app)


@app.errorhandler(CSRFError)
def handle_csrf_error(e):
    return '<h1>Stupid Hacker!</h1>', 400


@app.before_request
def start():
    g.start = time()


@app.teardown_request
def close(exception):
    if exception is not None:
        logger.warning(exception)
    logger.info("Elapsed time: %.2fms" % ((time() - g.start) * 1000))


if __name__ == '__main__':
    init(logger)
    app.register_blueprint(UserSystem)
    app.register_blueprint(TicketSystem)
    app.register_blueprint(TrainSystem)
    app.register_blueprint(AdminSystem)
    app.run(threaded=True, debug=True)
