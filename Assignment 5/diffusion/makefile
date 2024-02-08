LIBRARIES = -L/usr/lib64/openmpi/lib -lm -lmpi -Wl,-rpath,/usr/lib64/openmpi/lib -Wl,--enable-new-dtags
CFLAGS  = -O3 -I. -I/usr/include/openmpi-x86_64 -pthread $(LIBRARIES)

.PHONY : all
all : $(BINS) 

diffusion_mpi : diffusion_mpi.c
	gcc $(CFLAGS) -o $@ $<

diffusion : diffusion.c
	gcc $(CFLAGS) -o $@ $<

clean :
	rm -rf diffusion diffusion.tar.gz
	rm -rf extracted/

tar :
	tar -czvf diffusion.tar.gz ./diffusion.c makefile

test :
	/home/hpc2023/diffusion_mpi/check_submission.jl /home/hpcuser128/Assignment4/mpi/diffusion.tar.gz

#/home/hpc2023/newton_iteration/check_submission.jl /home/hpcuser128/Assignment3/v1/newton.tar.gz