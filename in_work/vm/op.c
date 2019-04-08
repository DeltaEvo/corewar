/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   op.c                                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: prastoin <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/03/14 14:32:56 by prastoin          #+#    #+#             */
/*   Updated: 2019/04/08 09:32:54 by prastoin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "vm.h"

t_core_fcnt g_fcnt[17] = {
	[0x01] = live,
	[0x02] = ld,
	[0x03] = st,
	[0x04] = add,
	[0x05] = sub,
	[0x06] = ft_and,
	[0x07] = ft_or,
	[0x08] = ft_xor,
	[0x09] = zjmp,
	[0x0a] = ldi,
	[0x0b] = sti,
	[0x0c] = ft_fork,
	[0x0d] = lld,
	[0x0e] = lldi,
	[0x0f] = lfork,
	[0x10] = aff
};

