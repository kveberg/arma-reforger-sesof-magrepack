modded class SCR_PlayerController
{
	void CombineMags(MagazineComponent fromMag, MagazineComponent toMag)
	{
		if(!fromMag || !toMag)
		{
			Print("Could not locate magazine components", LogLevel.ERROR);
			return;
		}
		
		int fromCount = fromMag.GetAmmoCount();
		int toCount = toMag.GetAmmoCount();
		int maxCount = toMag.GetMaxAmmoCount();
		
		if(fromCount + toCount <= maxCount)
		{
			toMag.SetAmmoCount(fromCount + toCount);
			
			// From mag was fully exhausted 			
			SCR_EntityHelper.DeleteEntityAndChildren(fromMag.GetOwner());
		}
		else
		{
			int remainder = (fromCount + toCount) % maxCount;
			
			toMag.SetAmmoCount(maxCount);
			
			if(remainder > 0)
				fromMag.SetAmmoCount(remainder);
			else 
				SCR_EntityHelper.DeleteEntityAndChildren(fromMag.GetOwner());
		}	
	}	
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	
	void RpcAsk_CombineMags(RplId fromMagazineComponent, RplId toMagazineComponent)
	{
		if(!fromMagazineComponent.IsValid() || 
		   !toMagazineComponent.IsValid())
		{
			Print("Invalid magazine components");
			return;
		}
		
		MagazineComponent fromMag = MagazineComponent.Cast(Replication.FindItem(fromMagazineComponent));
		MagazineComponent toMag = MagazineComponent.Cast(Replication.FindItem(toMagazineComponent));
		
		CombineMags(fromMag, toMag);		
	}
};
