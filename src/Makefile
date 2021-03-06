
include Makefile.in
CFLAGS += -I$(DECOMP_DIR)

OBJECTS  = Filter.o MPI_3DCompact.o Utils.o Derivatives.o SpongeBC.o UniformCSolver_BC.o UniformCSolver_Core.o UniformCSolverConservative_Core.o UniformCSolverConservative_BC.o UniformCSolverConservativeYBase_Core.o UniformCSolverConservativeYBase_BC.o CurvilinearCSolver_Core.o CurvilinearCSolver_BC.o CurvilinearInterpolator.o 

default: MPI_3DCompact

MPI_3DCompact:
	@echo
	@echo "///////////////////////////////////////////"
	@echo " Making Pencil Decomposition Components... " 
	@echo "///////////////////////////////////////////"
	@echo
	make -C $(DECOMP_DIR)
	@echo
	@echo "///////////////////////////////////////////"
	@echo " Making Solver Components..." 
	@echo "///////////////////////////////////////////"
	@echo
	make MPI_3DCompact.exe 


MPI_3DCompact.exe: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ -I$(INC) $(OBJECTS) -L$(LIB) -L$(DECOMP_DIR) -lc2decomp $(LIBF) 

MPI_3DCompact.o: MPI_3DCompact.cpp Domain.hpp BC.hpp Utils.hpp Macros.hpp TimeStepping.hpp AbstractCSolver.hpp UniformCSolver.hpp UniformCSolverConservative.hpp AbstractRK.hpp CurvilinearCSolver.hpp TVDRK3.hpp AbstractSingleBlockMesh.hpp AlgebraicSingleBlockMesh.hpp 
	$(CC) $(CFLAGS) -c $<

Utils.o: Utils.cpp Domain.hpp Utils.hpp
	$(CC) $(CFLAGS) -c $<

Derivatives.o: Derivatives.cpp Derivatives.hpp Domain.hpp BC.hpp Utils.hpp Macros.hpp 
	$(CC) $(CFLAGS) -c $<

SpongeBC.o: SpongeBC.cpp SpongeBC.hpp Domain.hpp BC.hpp Macros.hpp IdealGas.hpp AbstractSingleBlockMesh.hpp Utils.hpp
	$(CC) $(CFLAGS) -c $<

Filter.o: Filter.cpp Filter.hpp Domain.hpp BC.hpp Utils.hpp Macros.hpp 
	$(CC) $(CFLAGS) -c $<

UniformCSolver_BC.o: UniformCSolver_BC.cpp UniformCSolver.hpp AbstractSingleBlockMesh.hpp Macros.hpp Utils.hpp SpongeBC.hpp AbstractCSolver.hpp PngWriter.hpp
	$(CC) $(CFLAGS) -c $<

UniformCSolver_Core.o: UniformCSolver_Core.cpp UniformCSolver.hpp Macros.hpp Utils.hpp SpongeBC.hpp AbstractCSolver.hpp PngWriter.hpp
	$(CC) $(CFLAGS) -c $<

UniformCSolverConservative_BC.o: UniformCSolverConservative_BC.cpp UniformCSolverConservative.hpp Macros.hpp Utils.hpp SpongeBC.hpp AbstractCSolver.hpp PngWriter.hpp
	$(CC) $(CFLAGS) -c $<

UniformCSolverConservative_Core.o: UniformCSolverConservative_Core.cpp UniformCSolverConservative.hpp Macros.hpp Utils.hpp SpongeBC.hpp AbstractCSolver.hpp PngWriter.hpp
	$(CC) $(CFLAGS) -c $<

UniformCSolverConservativeYBase_BC.o: UniformCSolverConservativeYBase_BC.cpp UniformCSolverConservativeYBase.hpp Macros.hpp Utils.hpp SpongeBC.hpp AbstractCSolver.hpp PngWriter.hpp
	$(CC) $(CFLAGS) -c $<

UniformCSolverConservativeYBase_Core.o: UniformCSolverConservativeYBase_Core.cpp UniformCSolverConservativeYBase.hpp Macros.hpp Utils.hpp SpongeBC.hpp AbstractCSolver.hpp PngWriter.hpp
	$(CC) $(CFLAGS) -c $<

CurvilinearCSolver_BC.o: CurvilinearCSolver_BC.cpp CurvilinearCSolver.hpp Macros.hpp Utils.hpp SpongeBC.hpp AbstractCSolver.hpp AlgebraicSingleBlockMesh.hpp PngWriter.hpp Adt.hpp
	$(CC) $(CFLAGS) -c $<

CurvilinearCSolver_Core.o: CurvilinearCSolver_Core.cpp CurvilinearCSolver.hpp Macros.hpp Utils.hpp SpongeBC.hpp AbstractCSolver.hpp AlgebraicSingleBlockMesh.hpp PngWriter.hpp Adt.hpp
	$(CC) $(CFLAGS) -c $<

CurvilinearInterpolator.o: CurvilinearInterpolator.cpp CurvilinearInterpolator.hpp Macros.hpp Utils.hpp AbstractCSolver.hpp AlgebraicSingleBlockMesh.hpp BC.hpp Domain.hpp
	$(CC) $(CFLAGS) -c $<

clean: 
	rm -rf   *.o *.exe *.a

superclean:
	make clean -C $(DECOMP_DIR)
	rm -r    *.o *.exe *.a


