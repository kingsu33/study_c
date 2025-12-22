# study_c

# docker build
docker build -t c-dev .
# docker run
docker run -it --name c-dev-container -v ${PWD}:/app c-dev