#include <math.h>

#define MAX(a, b) (a) > (b) ? (a) : (b)
#define NORM(x, y, z) sqrt((x)*(x)+(y)*(y)+(z)*(z))
#define DOT(ax,ay,az,bx,by,bz) ((ax)*(bx)+(ay)*(by)+(az)*(bz))

extern "C" {
void build_kernel(
        double *data,
        const void *indices,
        const void *indptr,
        int index_stride,
        size_t rows,
        const double *train_x,
        const double *x,
        double l_pos,
        double power)
{
        const size_t stride = 6;

#pragma omp parallel for num_threads(39)
        for (size_t row = 0; row < rows; row++) {
                size_t cols = index_stride==4 ? ((const int*)indptr)[row] : ((const long long *)indptr)[row];
                size_t cole = index_stride==4 ? ((const int*)indptr)[row+1] : ((const long long *)indptr)[row+1];
                for (size_t i = cols; i < cole; i++) {
                        size_t col = index_stride==4 ? ((const int*)indices)[i] : ((const long long *)indices)[i];
                        double view_dist;
                        double kernel_pos, kernel_view;

                        kernel_pos = exp(-NORM(
                                train_x[stride*col]   - x[stride*row],
                                train_x[stride*col+1] - x[stride*row+1],
                                train_x[stride*col+2] - x[stride*row+2]
                        )/l_pos);
                        kernel_pos = pow(kernel_pos, 0.6);

                        view_dist = 1. - DOT(
                                train_x[stride*col+3], 
                                train_x[stride*col+4], 
                                train_x[stride*col+5],
                                x[stride*row+3],
                                x[stride*row+4],
                                x[stride*row+5]);
                        // kernel_view = pow(1.-view_dist, power);
                        
                        kernel_view = exp(-view_dist/0.2);
                        kernel_view = pow(kernel_view, 1.6);

                        data[i] = kernel_pos * kernel_view;
                }
        }
}
}