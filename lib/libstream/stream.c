//*************************************************************************
// Authors: Oleksiy Kebkal                                                *
//*************************************************************************

// ISO C headers.
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wordexp.h>

#include <stream.h>
// Declare driver initialization functions.
#define STREAM(a) \
    int stream_impl_ ## a ## _new(stream_t*);
#include "stream.def"

static const char* s_drivers[] = {
#define STREAM(a) #a,
#include "stream.def"
    NULL
};

const char** stream_get_drivers(void)
{
    return s_drivers;
}

stream_t *stream_new_v(int direction, const char* driver, const char* args)
{
    stream_t *stream = (stream_t*)calloc(1, sizeof(stream_t));
    int rv = 0;
    wordexp_t wbuf;

    /* Need to expand ~ after <driver>: prefix */
    wordexp(args, &wbuf, WRDE_NOCMD);
    if (wbuf.we_wordc == 1)
        strcpy(stream->args, wbuf.we_wordv[0]);
    else
        strcpy(stream->args, args);
    wordfree(&wbuf);

    stream->sample_size = sizeof(uint16_t);
    stream->direction = direction;
    stream_set_fs(stream, 62500);

// Select appropriate driver.
#define STREAM(a)                                                      \
    if (strcmp(#a, driver) == 0) {rv = stream_impl_ ## a ## _new(stream);} else
#include "stream.def"
    {rv = STREAM_ERROR;}

    if (rv != STREAM_ERROR_NONE) {
        free(stream);
        return NULL;
    }
    return stream;
}

void stream_free(stream_t *stream)
{
    if (stream->free)
        stream->free(stream);
    free(stream);
    stream = NULL;
}

void stream_set_fs(stream_t *stream, unsigned fs)
{
    stream->fs = fs;
}

unsigned stream_get_fs(stream_t *stream)
{
    return stream->fs;
}

unsigned stream_get_sample_size(stream_t *stream)
{
    return stream->sample_size;
}

int stream_open(stream_t *stream)
{
    return stream->open(stream);
}

int stream_openfp(stream_t *stream, FILE *fp)
{
    if (stream->openfp)
        return stream->openfp(stream, fp);
    return STREAM_ERROR;
}

int stream_close(stream_t *stream)
{
    return stream->close(stream);
}

int stream_read(stream_t *stream, uint16_t* samples, unsigned sample_count)
{
    if (stream) {
        return stream->read(stream, samples, sample_count);
    } else {
        return STREAM_ERROR;
    }
}

int stream_write(stream_t *stream, uint16_t *samples, unsigned sample_count)
{
    if (stream) {
        return stream->write(stream, samples, sample_count);
    } else {
        return STREAM_ERROR;
    }
}

ssize_t stream_count(stream_t *stream)
{
    return stream->count(stream);
}

int stream_get_errno(stream_t *stream)
{
    return stream->get_errno(stream);
}

const char* stream_strerror(stream_t *stream)
{
    sprintf(stream->bfr_error, "%s \"%s\": %s", stream->get_error_op(stream)
            , stream->args, stream->strerror(stream));
    return stream->bfr_error;
}

const char* stream_get_name(stream_t *stream)
{
    if (!stream)
        return "No stream";
    return stream->name;
}

const char* stream_get_args(stream_t *stream)
{
    if (!stream)
        return "No stream";
    return stream->args;
}

/****************** streams_t ***********************/
stream_t* stream_new(int direction, char *description)
{
    char *arg = strdup(description);
    char *default_drv = "ascii";
    char *drv, *drv_param;
    stream_t *stream;

    if (strchr(arg, ':')) {
        drv = strtok(arg, ":");
        if (drv == NULL) {
            /* logger(ERR_LOG, "Output format error: %s\n", arg); */
            goto stream_new_error;
        }
        drv_param = strtok(NULL, "");
        if (drv_param == NULL) {
            /* logger(ERR_LOG, "Output description undefined\n"); */
            goto stream_new_error;
        }
    } else {
        char *ext = strrchr(arg, '.');

        if (!ext)
            drv = default_drv;
        else if (!strcmp(ext, ".dat") || !strcmp(ext, ".txt"))
            drv = "ascii";
        else if (!strcmp(ext, ".raw") || !strcmp(ext, ".bin")
              || !strcmp(ext, ".dmp") || !strcmp(ext, ".fifo"))
            drv = "raw";
        else
            drv = default_drv;
        drv_param = arg;
    }
    
    stream = stream_new_v(direction, drv, drv_param);
    if (stream == NULL) {
        /* logger(ERR_LOG, "Stream creation error\n"); */
        goto stream_new_error;
    }

    free(arg);
    return stream;
stream_new_error:
    free(arg);
    return NULL;
}

stream_t* streams_add_new(streams_t *streams, int direction, char *description)
{
    stream_t *stream;

    if (streams->count >= STREAMS_MAX) {
        /* logger(ERR_LOG, "Too many streams open: %d\n", streams->count); */
        return NULL;
    }

    stream = stream_new(direction, description);

    if (!stream)
        return NULL;

    if (streams_add(streams, stream) < 0)
        return NULL;

    return stream;
}

int streams_add(streams_t *streams, stream_t *stream)
{
    if (streams->count >= STREAMS_MAX)
        return STREAM_ERROR;
    streams->streams[streams->count++] = stream;
    return STREAM_ERROR_NONE;
}

void streams_clean(streams_t *streams)
{
    int i;
    for (i = streams->count - 1; i >= 0 ; i--)
        streams_remove(streams, i);
}

// FIXME: need heavy testing!!!!!!!!!!!!!
int streams_remove(streams_t *streams, unsigned int index)
{
    if (index > streams->count)
        return STREAM_ERROR;

    if (streams->streams[index]) {
        stream_close(streams->streams[index]);
        stream_free (streams->streams[index]);
        if (index != streams->count) {
            memmove(&streams->streams[index], &streams->streams[index + 1], streams->count - index);
            streams->error_index = 0; // ??????????????????????????
        }
        streams->count--;
    }

    return STREAM_ERROR_NONE;
}

void stream_dump(stream_t *stream)
{
    if (!stream) {
        fprintf(stderr, "Stream Driver: NULL\n");
        return;
    }
    fprintf(stderr, "Stream Driver: %s\n", stream->name);
    fprintf(stderr, "Stream Driver Arguments: %s\n", stream->args);
    fprintf(stderr, "Stream Bytes Per Sample: %u\n", stream->sample_size);
    fprintf(stderr, "Stream Sampling Frequency (Hz): %u\n", stream->fs);
}

uint16_t* stream_load_samples(char *filename, size_t *len)
{
    int rc;
    uint16_t *samples = NULL;
    stream_t* stream;

    stream = stream_new(STREAM_INPUT, filename);

    if (!stream)
        return NULL;

    if (stream_open(stream) < 0) {
        stream_free(stream);
        return NULL;
    }

    rc = stream_count(stream);
    if (rc < 0)
        goto stream_load_samples_error;

    samples = malloc(rc * sizeof(uint16_t));
    if (!samples)
        goto stream_load_samples_error;

    rc = stream_read(stream, samples, rc);

    if (rc < 0) {
        free(samples);
        samples = NULL;
        goto stream_load_samples_error;
    }

    *len = rc;

stream_load_samples_error:
    stream_close(stream);
    stream_free(stream);

    return samples;
}
