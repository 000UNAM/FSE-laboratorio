#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CAPACIDAD 8
#define DATOS 100
#define PRODUCTOR_MS 100L
#define CONSUMIDOR_MS 300L

static int buffer[CAPACIDAD];
static int entrada, salida, ocupados, maximo;
static int producidos, consumidos, errores_fifo;
static sem_t huecos, llenos;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static struct timespec inicio;

static void sistema(int resultado, const char *operacion)
{
    if (resultado == -1) {
        perror(operacion);
        exit(EXIT_FAILURE);
    }
}

static void hilo(int error, const char *operacion)
{
    if (error != 0) {
        fprintf(stderr, "%s: %s\n", operacion, strerror(error));
        exit(EXIT_FAILURE);
    }
}

static void esperar(sem_t *semaforo)
{
    int r;
    do {
        r = sem_wait(semaforo);
    } while (r == -1 && errno == EINTR);
    sistema(r, "sem_wait");
}

static void dormir(long ms)
{
    struct timespec pausa = {ms / 1000, (ms % 1000) * 1000000L};
    struct timespec restante;
    int r;
    do {
        r = nanosleep(&pausa, &restante);
        if (r == -1 && errno == EINTR)
            pausa = restante;
    } while (r == -1 && errno == EINTR);
    sistema(r, "nanosleep");
}

static double tiempo_ms(void)
{
    struct timespec ahora;
    sistema(clock_gettime(CLOCK_MONOTONIC, &ahora), "clock_gettime");
    return (double)(ahora.tv_sec - inicio.tv_sec) * 1000.0
         + (double)(ahora.tv_nsec - inicio.tv_nsec) / 1000000.0;
}

static void *producir(void *arg)
{
    (void)arg;
    for (int dato = 1; dato <= DATOS; ++dato) {
        esperar(&huecos);
        hilo(pthread_mutex_lock(&mutex), "pthread_mutex_lock");
        buffer[entrada] = dato;
        entrada = (entrada + 1) % CAPACIDAD;
        ++ocupados;
        ++producidos;
        if (ocupados > maximo)
            maximo = ocupados;
        printf("%10.3f P %03d %d/%d\n", tiempo_ms(), dato,
               ocupados, CAPACIDAD);
        hilo(pthread_mutex_unlock(&mutex), "pthread_mutex_unlock");
        sistema(sem_post(&llenos), "sem_post");
        dormir(PRODUCTOR_MS);
    }
    return NULL;
}

static void *consumir(void *arg)
{
    (void)arg;
    for (int esperado = 1; esperado <= DATOS; ++esperado) {
        esperar(&llenos);
        hilo(pthread_mutex_lock(&mutex), "pthread_mutex_lock");
        int dato = buffer[salida];
        salida = (salida + 1) % CAPACIDAD;
        --ocupados;
        ++consumidos;
        if (dato != esperado)
            ++errores_fifo;
        printf("%10.3f C %03d %d/%d\n", tiempo_ms(), dato,
               ocupados, CAPACIDAD);
        hilo(pthread_mutex_unlock(&mutex), "pthread_mutex_unlock");
        sistema(sem_post(&huecos), "sem_post");
        dormir(CONSUMIDOR_MS);
    }
    return NULL;
}

int main(void)
{
    if (setvbuf(stdout, NULL, _IOLBF, 0) != 0) {
        fprintf(stderr, "No se pudo configurar la salida.\n");
        return EXIT_FAILURE;
    }
    sistema(sem_init(&huecos, 0, CAPACIDAD), "sem_init");
    sistema(sem_init(&llenos, 0, 0), "sem_init");
    sistema(clock_gettime(CLOCK_MONOTONIC, &inicio), "clock_gettime");
    printf("Buffer: %d espacios; datos: %d\n", CAPACIDAD, DATOS);
    printf("P = productor (%ld ms); C = consumidor (%ld ms)\n",
           PRODUCTOR_MS, CONSUMIDOR_MS);
    printf("Tiempo(ms) Tipo Dato Buffer\n");

    pthread_t productor, consumidor;
    hilo(pthread_create(&productor, NULL, producir, NULL), "pthread_create");
    hilo(pthread_create(&consumidor, NULL, consumir, NULL), "pthread_create");
    hilo(pthread_join(productor, NULL), "pthread_join");
    hilo(pthread_join(consumidor, NULL), "pthread_join");
    double duracion = tiempo_ms();

    sistema(sem_destroy(&huecos), "sem_destroy");
    sistema(sem_destroy(&llenos), "sem_destroy");
    hilo(pthread_mutex_destroy(&mutex), "pthread_mutex_destroy");
    int correcto = producidos == DATOS && consumidos == DATOS
                && errores_fifo == 0 && ocupados == 0
                && maximo <= CAPACIDAD;
    printf("\nResumen\nProducidos: %d\nConsumidos: %d\n", producidos, consumidos);
    printf("Errores de orden FIFO: %d\n", errores_fifo);
    printf("Ocupacion maxima: %d/%d\n", maximo, CAPACIDAD);
    printf("Elementos pendientes: %d\n", ocupados);
    printf("Tiempo total: %.3f ms\nVerificacion: %s\n", duracion,
           correcto ? "CORRECTA" : "FALLO");
    sistema(fflush(stdout), "fflush");
    return correcto ? EXIT_SUCCESS : EXIT_FAILURE;
}
