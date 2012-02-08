/* ngx_http_simple_example */
/* Author: Lucas Brasilino <lucas.brasilino@gmail.com> */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

/* Prototypes */

static char *ngx_http_simple_example(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_http_simple_example_handler(ngx_http_request_t *r);
static void *ngx_http_simple_example_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_simple_example_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);

/* Module configuration */

typedef struct {
  ngx_flag_t addlm;
  ngx_str_t respbody;
} ngx_http_simple_example_loc_conf_t;

static ngx_command_t  ngx_http_simple_example_commands[] = {
    { ngx_string("simple_example"),
      NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
      ngx_http_simple_example,
      0,
      0,
      NULL },
    { ngx_string("simple_example_content"),
      NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_simple_example_loc_conf_t, respbody),
      NULL },
    { ngx_string("simple_example_last_modified"),
      NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_flag_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_simple_example_loc_conf_t, addlm),
      NULL },
      ngx_null_command
};

static ngx_http_module_t  ngx_http_simple_example_module_ctx = {
    NULL,                          /* preconfiguration */
    NULL,                          /* postconfiguration */

    NULL,                          /* create main configuration */
    NULL,                          /* init main configuration */

    NULL,                          /* create server configuration */
    NULL,                          /* merge server configuration */

    ngx_http_simple_example_create_loc_conf,  /* create location configuration */
    ngx_http_simple_example_merge_loc_conf  /* merge location configuration */
};

ngx_module_t  ngx_http_simple_example_module = {
    NGX_MODULE_V1,
    &ngx_http_simple_example_module_ctx, /* module context */
    ngx_http_simple_example_commands,   /* module directives */
    NGX_HTTP_MODULE,               /* module type */
    NULL,                          /* init master */
    NULL,                          /* init module */
    NULL,                          /* init process */
    NULL,                          /* init thread */
    NULL,                          /* exit thread */
    NULL,                          /* exit process */
    NULL,                          /* exit master */
    NGX_MODULE_V1_PADDING
};

static char *
ngx_http_simple_example(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t  *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_simple_example_handler;

    return NGX_CONF_OK;
}

static ngx_int_t
ngx_http_simple_example_handler(ngx_http_request_t *r)
{
    ngx_int_t     rc;
    ngx_buf_t    *b;
    ngx_chain_t   out;
    ngx_str_t mimetype = { sizeof("text/html") -1, (u_char *)"text/html" };
    ngx_http_simple_example_loc_conf_t *conf;
    
    conf = ngx_http_get_module_loc_conf(r,ngx_http_simple_example_module); 

    if (!(r->method & NGX_HTTP_GET)) {
        return NGX_HTTP_NOT_ALLOWED;
    }

    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_type = mimetype;
    r->headers_out.content_length_n = conf->respbody.len;

    if (conf->addlm) {
      r->headers_out.last_modified_time = ngx_time();
    }     

    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
    if (b == NULL) {
	ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "Failed to allocate response buffer.");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    b->pos = conf->respbody.data;
    b->last = conf->respbody.data + conf->respbody.len;
    
    b->memory = 1;
    b->last_buf = 1;

    out.buf = b;
    out.next = NULL;

    rc = ngx_http_send_header(r);

    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }
    return ngx_http_output_filter(r,&out);
};

static void *
ngx_http_simple_example_create_loc_conf(ngx_conf_t *cf)
{
  ngx_http_simple_example_loc_conf_t *conf;

  conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_simple_example_loc_conf_t));
  if (conf == NULL) {
    return NGX_CONF_ERROR;
  }
  
  conf->addlm = NGX_CONF_UNSET;
  return conf;
}

static char *
ngx_http_simple_example_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
  ngx_http_simple_example_loc_conf_t *prev = parent;
  ngx_http_simple_example_loc_conf_t *conf = child;

  ngx_conf_merge_str_value (conf->respbody, prev->respbody,
			    "<html><head><title>Simple Example</title></head>\n"
			    "<body><h1>Default Page</h1>"
			    "<p><tt>simple_page_content</tt> not set</p>"
			    "</body>\n"
			    "</html>");
  ngx_conf_merge_value (conf->addlm, prev->addlm, 0);
  
  return NGX_CONF_OK;
}

