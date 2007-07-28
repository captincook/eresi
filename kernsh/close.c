/*
** close.c for kernsh
** 
** $Id: close.c,v 1.1 2007-07-28 15:02:23 pouik Exp $
**
*/
#include "kernsh.h"
#include "libkernsh.h"

/* Close the memory device */
int		cmd_closemem()
{
  int		ret;
  char		buff[BUFSIZ];
  char          logbuf[BUFSIZ];
  time_t        uloadt;

  PROFILER_IN(__FILE__, __FUNCTION__, __LINE__);

#if defined(USE_READLN)
  rl_callback_handler_remove();
#endif

  /* Close memory */
  ret = kernsh_closemem();

  if (ret)
    PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__, 
		 "Cannot close memory", -1);

  if (libkernshworld.open_static)
    {
      /* Rip from cmd_unload */

      /* Do not unload dependences of files or objects with linkmap entry */
      if (hash_size(&libkernshworld.root->parent_hash))
	PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__,
		     "Unload parent object first", -1);
      if (libkernshworld.root->linkmap)
	PROFILER_ERR(__FILE__, __FUNCTION__, __LINE__,
		     "You cannot unload a mapped object", -1);
      ret = revm_unload_dep(libkernshworld.root, libkernshworld.root);
      if (!world.state.revm_quiet)
	{
	  time(&uloadt);
	  snprintf(logbuf, BUFSIZ - 1, "%s [*] Object %-40s unloaded on %s \n",
		   (ret ? "" : "\n"), libkernshworld.root->name, ctime(&uloadt));
	  revm_output(logbuf);
	}
      
      /* Clean various hash tables of this binary entry and return OK */
      hash_del(&file_hash, libkernshworld.root->name);
      if (hash_get(&world.shared_hash, libkernshworld.root->name))
	hash_del(&world.shared_hash, libkernshworld.root->name);
      else
	hash_del(&world.curjob->loaded, libkernshworld.root->name);
      elfsh_unload_obj(libkernshworld.root);

      libkernshworld.open_static = 0;
    }
    
  memset(buff, '\0', sizeof(buff));
  snprintf(buff, sizeof(buff), 
	   "%s\n\n",
	   revm_colorfieldstr("[+] CLOSE MEMORY"));
  revm_output(buff);
  revm_endline();

#if defined(USE_READLN)
  rl_callback_handler_install(vm_get_prompt(), vm_ln_handler);
  readln_column_update();
#endif

  PROFILER_ROUT(__FILE__, __FUNCTION__, __LINE__, 0);
}