/*
 * (C) 2007 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 */

#include "internal/internal.h"

static int __cmp(int attr,
		 const struct nf_conntrack *ct1, 
		 const struct nf_conntrack *ct2,
		 unsigned int flags,
		 int (*cmp)(const struct nf_conntrack *ct1,
		 	    const struct nf_conntrack *ct2,
			    unsigned int flags))
{
	if (test_bit(attr, ct1->set) && test_bit(attr, ct2->set)) {
		return cmp(ct1, ct2, flags);
	} else if (flags & NFCT_CMP_MASK &&
		   test_bit(attr, ct1->set)) {
		return 0;
	} else if (flags & NFCT_CMP_STRICT) {
		return 0;
	}
	return 1;
}

static int
cmp_orig_l3proto(const struct nf_conntrack *ct1,
		 const struct nf_conntrack *ct2,
		 unsigned int flags)
{
	return (ct1->tuple[__DIR_ORIG].l3protonum ==
		ct2->tuple[__DIR_ORIG].l3protonum);
}

static int
cmp_icmp_id(const struct nf_conntrack *ct1,
	    const struct nf_conntrack *ct2,
	    unsigned int flags)
{
	return (ct1->tuple[__DIR_ORIG].l4src.icmp.id ==
		ct2->tuple[__DIR_ORIG].l4src.icmp.id);
}

static int
cmp_icmp_type(const struct nf_conntrack *ct1,
	      const struct nf_conntrack *ct2,
	      unsigned int flags)
{
	return (ct1->tuple[__DIR_ORIG].l4dst.icmp.type ==
		ct2->tuple[__DIR_ORIG].l4dst.icmp.type);
}

static int
cmp_icmp_code(const struct nf_conntrack *ct1,
	      const struct nf_conntrack *ct2,
	      unsigned int flags)
{
	return (ct1->tuple[__DIR_ORIG].l4dst.icmp.code ==
		ct2->tuple[__DIR_ORIG].l4dst.icmp.code);
}

static int
cmp_orig_port_src(const struct nf_conntrack *ct1,
		  const struct nf_conntrack *ct2,
		  unsigned int flags)
{
	return (ct1->tuple[__DIR_ORIG].l4src.all ==
		ct2->tuple[__DIR_ORIG].l4src.all);
}

static int
cmp_orig_port_dst(const struct nf_conntrack *ct1,
		  const struct nf_conntrack *ct2,
		  unsigned int flags)
{
	return (ct1->tuple[__DIR_ORIG].l4dst.all ==
		ct2->tuple[__DIR_ORIG].l4dst.all);
}

static int 
cmp_orig_l4proto(const struct nf_conntrack *ct1,
		 const struct nf_conntrack *ct2,
		 unsigned int flags)
{
	if (ct1->tuple[__DIR_ORIG].protonum != ct2->tuple[__DIR_ORIG].protonum)
		return 0;

	switch(ct1->tuple[__DIR_ORIG].protonum) {
	case IPPROTO_ICMP:
	case IPPROTO_ICMPV6:
		if (!__cmp(ATTR_ICMP_ID, ct1, ct2, flags, cmp_icmp_id))
			return 0;
		if (!__cmp(ATTR_ICMP_CODE, ct1, ct2, flags, cmp_icmp_code))
			return 0;
		if (!__cmp(ATTR_ICMP_TYPE, ct1, ct2, flags, cmp_icmp_type))
			return 0;
		break;
	case IPPROTO_TCP:
	case IPPROTO_UDP:
	case IPPROTO_UDPLITE:
	case IPPROTO_DCCP:
	case IPPROTO_SCTP:
		if (!__cmp(ATTR_ORIG_PORT_SRC, ct1, ct2, 
			       flags, cmp_orig_port_src))
			return 0;
		if (!__cmp(ATTR_ORIG_PORT_DST, ct1, ct2,
			       flags, cmp_orig_port_dst))
			return 0;
		break;
	}
	return 1;
}

static int 
cmp_orig_ipv4_src(const struct nf_conntrack *ct1,
		  const struct nf_conntrack *ct2,
		  unsigned int flags)
{
	return (ct1->tuple[__DIR_ORIG].src.v4 == ct2->tuple[__DIR_ORIG].src.v4);}

static int 
cmp_orig_ipv4_dst(const struct nf_conntrack *ct1,
		  const struct nf_conntrack *ct2,
		  unsigned int flags)
{
	return (ct1->tuple[__DIR_ORIG].dst.v4 == ct2->tuple[__DIR_ORIG].dst.v4);}

static int 
cmp_orig_ipv6_src(const struct nf_conntrack *ct1,
		  const struct nf_conntrack *ct2,
		  unsigned int flags)
{
	return (memcmp(&ct1->tuple[__DIR_ORIG].src.v6,
		&ct2->tuple[__DIR_ORIG].src.v6,
		sizeof(struct in6_addr)) == 0);
}

static int 
cmp_orig_ipv6_dst(const struct nf_conntrack *ct1,
		  const struct nf_conntrack *ct2,
		  unsigned int flags)
{
	return (memcmp(&ct1->tuple[__DIR_ORIG].dst.v6,
		&ct2->tuple[__DIR_ORIG].dst.v6,
		sizeof(struct in6_addr)) == 0);
}

static int cmp_orig(const struct nf_conntrack *ct1,
		    const struct nf_conntrack *ct2,
		    unsigned int flags)
{
	if (!__cmp(ATTR_ORIG_L3PROTO, ct1, ct2, flags, cmp_orig_l3proto))
		return 0;
	if (!__cmp(ATTR_ORIG_L4PROTO, ct1, ct2, flags, cmp_orig_l4proto))
		return 0;
	if (!__cmp(ATTR_ORIG_IPV4_SRC, ct1, ct2, flags, cmp_orig_ipv4_src))
		return 0;
	if (!__cmp(ATTR_ORIG_IPV4_DST, ct1, ct2, flags, cmp_orig_ipv4_dst))
		return 0;
	if (!__cmp(ATTR_ORIG_IPV6_SRC, ct1, ct2, flags, cmp_orig_ipv6_src))
		return 0;
	if (!__cmp(ATTR_ORIG_IPV6_DST, ct1, ct2, flags, cmp_orig_ipv6_dst))
		return 0;

	return 1;
}

static int
cmp_repl_l3proto(const struct nf_conntrack *ct1,
		 const struct nf_conntrack *ct2,
		 unsigned int flags)
{
	return (ct1->tuple[__DIR_REPL].l3protonum ==
		ct2->tuple[__DIR_REPL].l3protonum);
}

static int
cmp_repl_port_src(const struct nf_conntrack *ct1,
		  const struct nf_conntrack *ct2,
		  unsigned int flags)
{
	return (ct1->tuple[__DIR_REPL].l4src.all ==
		ct2->tuple[__DIR_REPL].l4src.all);
}

static int
cmp_repl_port_dst(const struct nf_conntrack *ct1,
		  const struct nf_conntrack *ct2,
		  unsigned int flags)
{
	return (ct1->tuple[__DIR_REPL].l4dst.all ==
		ct2->tuple[__DIR_REPL].l4dst.all);
}

static int 
cmp_repl_l4proto(const struct nf_conntrack *ct1,
		 const struct nf_conntrack *ct2,
		 unsigned int flags)
{
	if (ct1->tuple[__DIR_REPL].protonum != ct2->tuple[__DIR_REPL].protonum)
		return 0;

	switch(ct1->tuple[__DIR_REPL].protonum) {
	case IPPROTO_ICMP:
	case IPPROTO_ICMPV6:
		if (!__cmp(ATTR_ICMP_ID, ct1, ct2, flags, cmp_icmp_id))
			return 0;
		if (!__cmp(ATTR_ICMP_CODE, ct1, ct2, flags, cmp_icmp_code))
			return 0;
		if (!__cmp(ATTR_ICMP_TYPE, ct1, ct2, flags, cmp_icmp_type))
			return 0;
		break;
	case IPPROTO_TCP:
	case IPPROTO_UDP:
	case IPPROTO_UDPLITE:
	case IPPROTO_DCCP:
	case IPPROTO_SCTP:
		if (!__cmp(ATTR_REPL_PORT_SRC, ct1, ct2, 
			       flags, cmp_repl_port_src))
			return 0;
		if (!__cmp(ATTR_REPL_PORT_DST, ct1, ct2,
			       flags, cmp_repl_port_dst))
			return 0;
		break;
	}
	return 1;
}

static int 
cmp_repl_ipv4_src(const struct nf_conntrack *ct1,
		  const struct nf_conntrack *ct2,
		  unsigned int flags)
{
	return (ct1->tuple[__DIR_REPL].src.v4 == ct2->tuple[__DIR_REPL].src.v4);}

static int 
cmp_repl_ipv4_dst(const struct nf_conntrack *ct1,
		  const struct nf_conntrack *ct2,
		  unsigned int flags)
{
	return (ct1->tuple[__DIR_REPL].dst.v4 == ct2->tuple[__DIR_REPL].dst.v4);}

static int 
cmp_repl_ipv6_src(const struct nf_conntrack *ct1,
		  const struct nf_conntrack *ct2,
		  unsigned int flags)
{
	return (memcmp(&ct1->tuple[__DIR_REPL].src.v6,
		&ct2->tuple[__DIR_REPL].src.v6,
		sizeof(struct in6_addr)) == 0);
}

static int 
cmp_repl_ipv6_dst(const struct nf_conntrack *ct1,
		  const struct nf_conntrack *ct2,
		  unsigned int flags)
{
	return (memcmp(&ct1->tuple[__DIR_REPL].dst.v6,
		&ct2->tuple[__DIR_REPL].dst.v6,
		sizeof(struct in6_addr)) == 0);
}

static int cmp_repl(const struct nf_conntrack *ct1,
		    const struct nf_conntrack *ct2,
		    unsigned int flags)
{
	if (!__cmp(ATTR_REPL_L3PROTO, ct1, ct2, flags, cmp_repl_l3proto))
		return 0;
	if (!__cmp(ATTR_REPL_L4PROTO, ct1, ct2, flags, cmp_repl_l4proto))
		return 0;
	if (!__cmp(ATTR_REPL_IPV4_SRC, ct1, ct2, flags, cmp_repl_ipv4_src))
		return 0;
	if (!__cmp(ATTR_REPL_IPV4_DST, ct1, ct2, flags, cmp_repl_ipv4_dst))
		return 0;
	if (!__cmp(ATTR_REPL_IPV6_SRC, ct1, ct2, flags, cmp_repl_ipv6_src))
		return 0;
	if (!__cmp(ATTR_REPL_IPV6_DST, ct1, ct2, flags, cmp_repl_ipv6_dst))
		return 0;

	return 1;
}

static int 
cmp_id(const struct nf_conntrack *ct1,
       const struct nf_conntrack *ct2,
       unsigned int flags)
{
	return (ct1->id == ct2->id);
}

static int 
cmp_mark(const struct nf_conntrack *ct1,
	 const struct nf_conntrack *ct2,
	 unsigned int flags)
{
	return (ct1->mark == ct2->mark);
}

static int 
cmp_timeout(const struct nf_conntrack *ct1,
	    const struct nf_conntrack *ct2,
	    unsigned int flags)
{
	int ret = 0;

#define __NFCT_CMP_TIMEOUT (NFCT_CMP_TIMEOUT_LE | NFCT_CMP_TIMEOUT_GT)

	if (!(flags & __NFCT_CMP_TIMEOUT) &&
	    ct1->timeout != ct2->timeout)
	    	return 0;
	else {
		if (flags & NFCT_CMP_TIMEOUT_GT &&
		    ct1->timeout > ct2->timeout)
			ret = 1;
		else if (flags & NFCT_CMP_TIMEOUT_LT &&
			 ct1->timeout < ct2->timeout)
		    	ret = 1;
		else if (flags & NFCT_CMP_TIMEOUT_EQ &&
			 ct1->timeout == ct2->timeout)
			ret = 1;

	    	if (ret == 0)
			return 0;
	}
	return ret;
}

static int 
cmp_status(const struct nf_conntrack *ct1,
	   const struct nf_conntrack *ct2,
	   unsigned int flags)
{
	return ((ct1->status & ct2->status) == ct1->status);
}

static int 
cmp_tcp_state(const struct nf_conntrack *ct1,
	      const struct nf_conntrack *ct2,
	      unsigned int flags)
{
	return (ct1->protoinfo.tcp.state == ct2->protoinfo.tcp.state);
}

static int 
cmp_sctp_state(const struct nf_conntrack *ct1,
	       const struct nf_conntrack *ct2,
	       unsigned int flags)
{
	return (ct1->protoinfo.sctp.state == ct2->protoinfo.sctp.state);
}

static int
cmp_dccp_state(const struct nf_conntrack *ct1,
	       const struct nf_conntrack *ct2,
	       unsigned int flags)
{
	return (ct1->protoinfo.dccp.state == ct2->protoinfo.dccp.state);
}

static int 
cmp_zone(const struct nf_conntrack *ct1,
	 const struct nf_conntrack *ct2,
	 unsigned int flags)
{
	return (ct1->zone == ct2->zone);
}

static int cmp_meta(const struct nf_conntrack *ct1,
		    const struct nf_conntrack *ct2,
		    unsigned int flags)
{
	if (!__cmp(ATTR_ID, ct1, ct2, flags, cmp_id))
		return 0;
	if (!__cmp(ATTR_MARK, ct1, ct2, flags, cmp_mark))
		return 0;
	if (!__cmp(ATTR_TIMEOUT, ct1, ct2, flags, cmp_timeout))
		return 0;
	if (!__cmp(ATTR_STATUS, ct1, ct2, flags, cmp_status))
		return 0;
	if (!__cmp(ATTR_TCP_STATE, ct1, ct2, flags, cmp_tcp_state))
		return 0;
	if (!__cmp(ATTR_SCTP_STATE, ct1, ct2, flags, cmp_sctp_state))
		return 0;
	if (!__cmp(ATTR_DCCP_STATE, ct1, ct2, flags, cmp_dccp_state))
		return 0;
	if (!__cmp(ATTR_ZONE, ct1, ct2, flags, cmp_zone))
		return 0;

	return 1;
}

int __compare(const struct nf_conntrack *ct1,
	      const struct nf_conntrack *ct2,
	      unsigned int flags)
{
	if ((flags & ~(NFCT_CMP_MASK|NFCT_CMP_STRICT)) == NFCT_CMP_ALL)
		return cmp_meta(ct1, ct2, flags) &&
		       cmp_orig(ct1, ct2, flags) &&
		       cmp_repl(ct1, ct2, flags);

	if (flags & NFCT_CMP_ORIG && !cmp_orig(ct1, ct2, flags))
		return 0;

	if (flags & NFCT_CMP_REPL && !cmp_repl(ct1, ct2, flags))
		return 0;

	return 1;
}
