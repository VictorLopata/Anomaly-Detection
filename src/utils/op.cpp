using namespace std;


void update_cov(int& n, double& avgX, double& avgY, double& cov, double newX, double newY) {
    n += 1;

    double deltaX = newX - avgX;
    double deltaY = newY - avgY;

    // Update averages
    avgX += deltaX / n;
    avgY += deltaY / n;

    // Update covariances
    cov += (deltaX * deltaY - cov) / n;
}