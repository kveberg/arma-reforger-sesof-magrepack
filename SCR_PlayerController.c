modded class SCR_PlayerController
{
	void CombineMags(MagazineComponent fromMag, MagazineComponent toMag, SCR_InventoryStorageManagerComponent managerComp)
	{
		if(!fromMag || !toMag)
		{
			Print("Could not locate magazine components", LogLevel.ERROR);
			return;
		}
		
		int fromCount = fromMag.GetAmmoCount();
		int toCount = toMag.GetAmmoCount();
		int maxCount = toMag.GetMaxAmmoCount();
		
		IEntity fromEntity = fromMag.GetOwner();
		IEntity toEntity = toMag.GetOwner();
		BaseInventoryStorageComponent storage = TW<BaseInventoryStorageComponent>.Find(managerComp.GetOwner());
		
		if(!storage)
		{
			Print("Was unable to locate storage or manager components", LogLevel.ERROR);
			return;
		}
		
		if(fromCount + toCount <= maxCount)
		{
			toMag.SetAmmoCount(fromCount + toCount);

			// Removing + Adding appears to make the magazine "stack" or "refresh"
			if(managerComp.TryRemoveItemFromInventory(toEntity, storage))
			{
				// Just print something if for whatever reason we can't add the magazine back
				if(!managerComp.TryInsertItemInStorage(toEntity, storage))
				{
					Print("Was unable to successfully add packed magazine back into storage", LogLevel.ERROR);
				}
			}
			
			// From mag was fully exhausted 			
			SCR_EntityHelper.DeleteEntityAndChildren(fromMag.GetOwner());
		}
		else
		{
			int remainder = (fromCount + toCount) % maxCount;
			
			toMag.SetAmmoCount(maxCount);

			// Removing + Adding appears to make the magazines "stack" or "refresh"
			if(managerComp.TryRemoveItemFromInventory(toEntity, storage))
			{
				// Just print something if for whatever reason we can't add the magazine back
				if(!managerComp.TryInsertItemInStorage(toEntity, storage))
				{
					Print("Was unable to successfully add packed magazine back into storage", LogLevel.ERROR);
				}
			}
			
			if(remainder > 0)
			{
				fromMag.SetAmmoCount(remainder);
				
				if(managerComp.TryRemoveItemFromInventory(fromEntity, storage))
				{
					if(!managerComp.TryInsertItemInStorage(fromEntity, storage))
					{
						Print("Was unable to successfully add packed magazine back into storage", LogLevel.ERROR);
					}
				}
			}				
			else 
				SCR_EntityHelper.DeleteEntityAndChildren(fromMag.GetOwner());
		}	
	}	
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_CombineMags(RplId fromMagazineComponent, RplId toMagazineComponent, RplId storageManagerComponent)
	{
		/*
  		    We just need the two magazines which need tweaking, along with the storage manager.
 		*/
		if(!fromMagazineComponent.IsValid())
		{
			Print("Invalid from magazine", LogLevel.ERROR);
			return;
		}
		
		if(!toMagazineComponent.IsValid())
		{
			Print("Invalid to magazine", LogLevel.ERROR);
			return;
		}
		
		if(!storageManagerComponent.IsValid())
		{
			Print("Invalid manager", LogLevel.ERROR);
			return;
		}
		
		MagazineComponent fromMag = MagazineComponent.Cast(Replication.FindItem(fromMagazineComponent));
		MagazineComponent toMag = MagazineComponent.Cast(Replication.FindItem(toMagazineComponent));
		SCR_InventoryStorageManagerComponent managerComponent = SCR_InventoryStorageManagerComponent.Cast(Replication.FindItem(storageManagerComponent));
		
		CombineMags(fromMag, toMag, managerComponent);		
	}
};
