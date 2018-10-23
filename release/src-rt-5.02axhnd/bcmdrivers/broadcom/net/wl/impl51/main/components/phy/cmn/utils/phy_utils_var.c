/*
 * PHY utils - nvram access functions.
 *
 * Copyright 2018 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: phy_utils_var.c 669287 2016-11-08 22:32:41Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmnvram.h>

#include <phy_utils_var.h>

#define MAX_FABID_CHARS	16

#if defined(BCMDBG)
char *
#ifdef BCMDBG
phy_utils_getvar(phy_info_t *pi, const char *name, const char *function)
#else
phy_utils_getvar(phy_info_t *pi, const char *name)
#endif // endif
{
#ifdef _MINOSL_
	return NULL;
#else
#ifdef BCMDBG
	/* Use vars pointing to itself as a flag that wlc_phy_attach is complete.
	 * Can't use NULL because it means there are no vars but we still
	 * need to potentially search NVRAM.
	 */

	if (pi->vars == (char *)&pi->vars)
		PHY_ERROR(("Usage of phy_utils_getvar() by %s after wlc_phy_attach\n", function));
#endif /* BCMDBG */

	ASSERT(pi->vars != (char *)&pi->vars);

	NVRAM_RECLAIM_CHECK(name);
	return phy_utils_getvar_internal(pi, name);
#endif	/* _MINOSL_ */
}

char *
phy_utils_getvar_internal(phy_info_t *pi, const char *name)
{
	char *vars = pi->vars;
	char *s;
	int len;

	if (!name)
		return NULL;

	len = strlen(name);
	if (len == 0)
		return NULL;

	/* first look in vars[] */
	for (s = vars; s && *s;) {
		if ((bcmp(s, name, len) == 0) && (s[len] == '='))
			return (&s[len+1]);

		while (*s++)
			;
	}

	/* Query nvram */
	return (nvram_get(name));

}
#else /* BCMDBG */
INLINE char *
phy_utils_getvar(phy_info_t *pi, const char *name)
{
#ifdef _MINOSL_
	return NULL;
#else
	return getvar(pi->vars, name);
#endif /* _MINOSL_ */
}
#endif /* BCMDBG */

char *
#ifdef BCMDBG
phy_utils_getvar_fabid(phy_info_t *pi, const char *name, const char *function)
#else
phy_utils_getvar_fabid(phy_info_t *pi, const char *name)
#endif // endif
{
	NVRAM_RECLAIM_CHECK(name);
#ifdef BCMDBG
	return phy_utils_getvar_fabid_internal(pi, name, function);
#else
	return phy_utils_getvar_fabid_internal(pi, name);
#endif // endif
}

static const char rstr_dotfabdot[] = ".fab.";
static const char rstr_SdotfabdotNd[] = "%s.fab.%d";

char *
#ifdef BCMDBG
phy_utils_getvar_fabid_internal(phy_info_t *pi, const char *name, const char *function)
#else
phy_utils_getvar_fabid_internal(phy_info_t *pi, const char *name)
#endif // endif
{
	char *val = NULL;

	if (pi->fabid) {
		uint16 sz = (strlen(name) + strlen(rstr_dotfabdot) + MAX_FABID_CHARS);
		/* freed in same function */
		char *fab_name = (char *) MALLOC_NOPERSIST(pi->sh->osh, sz);
		/* Prepare fab name */
		if (fab_name == NULL) {
			PHY_ERROR(("wl%d: %s: MALLOC failure\n",
				pi->sh->unit, __FUNCTION__));
			return FALSE;
		}
		snprintf(fab_name, sz, rstr_SdotfabdotNd, name, pi->fabid);

#ifdef BCMDBG
		val = phy_utils_getvar(pi, (const char *)fab_name, function);
#else
		val = phy_utils_getvar(pi, (const char *)fab_name);
#endif /* BCMDBG */
		MFREE(pi->sh->osh, fab_name, sz);
	}

	if (val == NULL) {
#ifdef BCMDBG
		val = phy_utils_getvar(pi, (const char *)name, function);
#else
		val = phy_utils_getvar(pi, (const char *)name);
#endif /* BCMDBG */
	}

	return val;
}

int
#ifdef BCMDBG
phy_utils_getintvar(phy_info_t *pi, const char *name, const char *function)
#else
phy_utils_getintvar(phy_info_t *pi, const char *name)
#endif // endif
{
	return phy_utils_getintvar_default(pi, name, 0);
}

int
phy_utils_getintvar_default(phy_info_t *pi, const char *name, int default_value)
{
#ifdef _MINOSL_
	return 0;
#else
	char *val = PHY_GETVAR(pi, name);
	if (val != NULL)
		return (bcm_strtoul(val, NULL, 0));

	return (default_value);
#endif	/* _MINOSL_ */
}

int
#ifdef BCMDBG
phy_utils_getintvararray(phy_info_t *pi, const char *name, int idx, const char *function)
#else
phy_utils_getintvararray(phy_info_t *pi, const char *name, int idx)
#endif // endif
{
	return PHY_GETINTVAR_ARRAY_DEFAULT(pi, name, idx, 0);
}

int
#ifdef BCMDBG
phy_utils_getintvararray_default(phy_info_t *pi, const char *name, int idx, int default_value,
	const char *function)
#else
phy_utils_getintvararray_default(phy_info_t *pi, const char *name, int idx, int default_value)
#endif // endif
{
	/* Rely on PHY_GETVAR() doing the NVRAM_RECLAIM_CHECK() */
	if (PHY_GETVAR(pi, name)) {
#ifdef BCMDBG
		return phy_utils_getintvararray_default_internal(pi, name, idx, default_value,
		                                                 function);
#else
		return phy_utils_getintvararray_default_internal(pi, name, idx, default_value);
#endif // endif
	} else return default_value;
}

int
#ifdef BCMDBG
phy_utils_getintvararray_default_internal(phy_info_t *pi, const char *name, int idx,
	int default_value, const char *function)
#else
phy_utils_getintvararray_default_internal(phy_info_t *pi, const char *name, int idx,
	int default_value)
#endif // endif
{
	int i, val;
	char *vars = pi->vars;
	char *fabspf = NULL;

	val = default_value;
	/* Check if fab specific values are present in the NVRAM
	*  if present, replace the regular value with the fab specific
	*/
	if (pi->fabid) {
		uint16 sz = (strlen(name) + strlen(rstr_dotfabdot) + MAX_FABID_CHARS);
		/* freed in same function */
		char *fab_name = (char *) MALLOC_NOPERSIST(pi->sh->osh, sz);
		/* Prepare fab name */
		if (fab_name == NULL) {
			PHY_ERROR(("wl%d: %s: MALLOC failure\n",
				pi->sh->unit, __FUNCTION__));
			return FALSE;
		}
		snprintf(fab_name, sz, rstr_SdotfabdotNd, name, pi->fabid);
		/* Get the value from fabid specific first if present
		*  Assumption: param.fab.<fabid>.fab.<fabid> never exist
		*/
		fabspf = PHY_GETVAR(pi, (const char *)fab_name);
		if (fabspf != NULL) {
			i = getintvararraysize(vars, fab_name);
			if ((i == 0) || (i <= idx))
				val = default_value;
			else
				val = getintvararray(vars, fab_name, idx);
		}
		MFREE(pi->sh->osh, fab_name, sz);
	}

	if (fabspf == NULL) {
		i = getintvararraysize(vars, name);
		if ((i == 0) || (i <= idx))
			val = default_value;
		else
			val = getintvararray(vars, name, idx);
	}
	return val;
}

#define PHY_SLICEVAR_GET(pi, a, m)	(PHY_GETVAR(pi, (a)) ? (a) : (m))

int phy_get_slicevar_name(phy_info_t *pi, const char *name, char **name_with_prefix_ref)
{
	uint16 sz;
	char* name_with_prefix = NULL;
	sz = (strlen(name) + strlen(pi->sh->vars_table_accessor)+1);
	name_with_prefix = (char *) MALLOC_NOPERSIST(pi->sh->osh, sz);
	/* Prepare fab name */
	if (name_with_prefix == NULL) {
		PHY_ERROR(("wl: %s: MALLOC failure\n", __FUNCTION__));
		return 0;
	}
	/* prefix accessor to the vars-name */
	name_with_prefix[0] = 0;
	bcmstrcat(name_with_prefix, pi->sh->vars_table_accessor);
	bcmstrcat(name_with_prefix, name);
	*name_with_prefix_ref = name_with_prefix;
	return sz;

}

void phy_slicevar_name_delete(phy_info_t *pi, char *name_with_prefix, uint16 sz)
{
	ASSERT(name_with_prefix);
	ASSERT(sz);

	MFREE(pi->sh->osh, name_with_prefix, sz);
}

int
#ifdef BCMDBG
phy_getintvar_slicespecific(phy_info_t *pi, const char *name, const char *function)
#else
phy_getintvar_slicespecific(phy_info_t *pi, const char *name)
#endif // endif
{
	int ret = 0;
#ifdef DUAL_PHY
	char *name_with_prefix = NULL;
	/* if accessor is initalized with slice/<slice_index>string */
	if (pi->sh->vars_table_accessor[0] !=  0) {
		uint16 sz = phy_get_slicevar_name(pi, name, &name_with_prefix);
		if (sz == 0)
		   return 0;
#ifdef BCMDBG
		ret = (int)phy_utils_getintvar(pi, PHY_SLICEVAR_GET(pi, name_with_prefix, name),
			function);
#else
		ret = (int)phy_utils_getintvar(pi, PHY_SLICEVAR_GET(pi, name_with_prefix, name));
#endif /* BCMDBG */
		phy_slicevar_name_delete(pi, name_with_prefix, sz);
	} else
#endif /* DUAL_PHY */
	{
#ifdef BCMDBG
		ret = (int)phy_utils_getintvar(pi, name, function);
#else
		ret = (int)phy_utils_getintvar(pi, name);
#endif /* BCMDBG */
	}
	return ret;
}

int
#ifdef BCMDBG
phy_utils_getintvararray_slicespecific(phy_info_t *pi,
	const char *name, int idx, const char *function)
#else
phy_utils_getintvararray_slicespecific(phy_info_t *pi, const char *name, int idx)
#endif // endif
{
#ifdef DUAL_PHY
	int ret;
	char *name_with_prefix = NULL;
	/* if accessor is initalized with slice/<slice_index>string */
	if (pi->sh->vars_table_accessor[0] !=  0) {
		uint16 sz = phy_get_slicevar_name(pi, name, &name_with_prefix);
		if (sz == 0)
		   return 0;
		ret = PHY_GETINTVAR_ARRAY_DEFAULT(pi, PHY_SLICEVAR_GET(pi, name_with_prefix, name),
		   idx, 0);
		phy_slicevar_name_delete(pi, name_with_prefix, sz);
		return ret;
	}
	else
#endif /* DUAL_PHY */
	{
		return PHY_GETINTVAR_ARRAY_DEFAULT(pi, name, idx, 0);
	}
}

int
phy_utils_getintvar_default_slicespecific(phy_info_t *pi, const char *name, int default_value)
{
#ifdef _MINOSL_
	return 0;
#else
	char *val = NULL;
#ifdef DUAL_PHY
	char *name_with_prefix = NULL;
	if (pi->sh->vars_table_accessor[0] !=  0) {
		uint16 sz = phy_get_slicevar_name(pi, name, &name_with_prefix);
		if (sz == 0)
		   return 0;

		val = PHY_GETVAR(pi, name_with_prefix);
		phy_slicevar_name_delete(pi, name_with_prefix, sz);
	} else
#endif /* DUAL_PHY */
	{
	  val = PHY_GETVAR(pi, name);
	}
	if (val != NULL)
	   return (bcm_strtoul(val, NULL, 0));
	return (default_value);
#endif	/* _MINOSL_ */
}
int
#ifdef BCMDBG
phy_utils_getintvararray_default_slicespecific(phy_info_t *pi,
	const char *name, int idx, int default_value, const char *function)
#else
phy_utils_getintvararray_default_slicespecific(phy_info_t *pi,
	const char *name, int idx, int default_value)
#endif // endif
{
	int val;
#ifdef DUAL_PHY
	char *name_with_prefix = NULL;
	if (pi->sh->vars_table_accessor[0] !=  0) {
		uint16 sz = phy_get_slicevar_name(pi, name, &name_with_prefix);
		if (sz == 0)
		   return 0;

	if (PHY_GETVAR(pi, name_with_prefix)) {
#ifdef BCMDBG
		val = phy_utils_getintvararray_default_internal(pi,
			name, idx, default_value, function);
#else
		//val = phy_utils_getintvararray_default_internal(pi, name, idx, default_value);
		val = PHY_GETINTVAR_ARRAY_DEFAULT(pi, PHY_SLICEVAR_GET(pi, name_with_prefix, name),
			idx, default_value);
#endif /* BCMDBG */
	} else {
		val = default_value;
	}
		phy_slicevar_name_delete(pi, name_with_prefix, sz);
	} else
#endif /* DUAL_PHY */
	{
		if (PHY_GETVAR(pi, name)) {
#ifdef BCMDBG
		val = phy_utils_getintvararray_default_internal(pi, name, idx, default_value,
			function);
#else
		val = phy_utils_getintvararray_default_internal(pi, name, idx, default_value);
#endif /* BCMDBG */
		} else {
			val = default_value;
		}
	}
	return val;
}

char *
#ifdef BCMDBG
phy_utils_getvar_fabid_slicespecific(phy_info_t *pi, const char *name, const char *function)
#else
phy_utils_getvar_fabid_slicespecific(phy_info_t *pi, const char *name)
#endif // endif
{
	char *ret = NULL;
	NVRAM_RECLAIM_CHECK(name);
#ifdef DUAL_PHY
	char *name_with_prefix = NULL;
	if (pi->sh->vars_table_accessor[0] !=  0) {
		uint16 sz = phy_get_slicevar_name(pi, name, &name_with_prefix);
		if (sz == 0)
		   return 0;

#ifdef BCMDBG
	ret = phy_utils_getvar_fabid_internal(pi, PHY_SLICEVAR_GET(pi, name_with_prefix, name),
		function);
#else
	ret =  phy_utils_getvar_fabid_internal(pi, PHY_SLICEVAR_GET(pi, name_with_prefix, name));
#endif /* BCMDBG */
	phy_slicevar_name_delete(pi, name_with_prefix, sz);
	} else
#endif /* DUAL_PHY */
	{
#ifdef BCMDBG
	ret = phy_utils_getvar_fabid_internal(pi, name, function);
#else
	ret = phy_utils_getvar_fabid_internal(pi, name);
#endif /* BCMDBG */
	}
	return ret;
}
