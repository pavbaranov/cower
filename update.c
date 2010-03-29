/*
 *  update.c
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdio.h>

#include <alpm.h>
#include <jansson.h>

#include "conf.h"
#include "download.h"
#include "package.h"
#include "search.h"
#include "util.h"

/** 
* @brief search for updates to foreign packages
* 
* @param foreign  an alpm_list_t of foreign pacman packages
* 
* @return number of updates available
*/
int aur_find_updates(alpm_list_t *foreign) {

  alpm_list_t *i;
  int ret = 0;

  /* Iterate over foreign packages */
  for (i = foreign; i; i = alpm_list_next(i)) {
    pmpkg_t *pmpkg = alpm_list_getdata(i);

    /* Do I exist in the AUR? */
    json_t *infojson = aur_rpc_query(AUR_RPC_QUERY_TYPE_INFO,
      alpm_pkg_get_name(pmpkg));

    if (infojson == NULL) { /* Not found, next candidate */
      json_decref(infojson);
      continue;
    }

    json_t *pkg;
    const char *remote_ver, *local_ver;

    pkg = json_object_get(infojson, "results");
    remote_ver = json_string_value(json_object_get(pkg, "Version"));
    local_ver = alpm_pkg_get_version(pmpkg);

    /* Version check */
    if (alpm_pkg_vercmp(remote_ver, local_ver) <= 0) {
      json_decref(infojson);
      continue;
    }

    ret++; /* Found an update, increment */
    if (config->op & OP_DL) /* -d found with -u */
      aur_get_tarball(infojson);
    else {
      if (config->color) {
        cprintf("%<%s%>", WHITE, alpm_pkg_get_name(pmpkg));
        if (! config->quiet)
          cprintf(" %<%s%> -> %<%s%>\n", GREEN, local_ver, GREEN, remote_ver);
      } else {
        if (! config->quiet)
          printf("%s %s -> %s\n", alpm_pkg_get_name(pmpkg), local_ver, remote_ver);
        else
          printf("%s\n", alpm_pkg_get_name(pmpkg));
      }
    }
  }

  return ret;
}

