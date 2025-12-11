void Timer(int m) {
    gameManager.update();
    glutPostRedisplay();
    glutTimerFunc(1000/60, Timer, 0);